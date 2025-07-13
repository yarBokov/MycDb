#ifndef __CONF_FILE_EDITOR_H
#define __CONF_FILE_EDITOR_H

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <cctype>

#include "libs/common/strings.hpp"
#include "libs/common/config.hpp"
#include "libs/dbcore/tx/isolation_level.h"

#include "server/core/exceptions.h"

#include <fmt/format.h>

namespace server::conf
{
    enum class conf_section_name
    {
        local,
        global
    };

    std::string section_name_to_string(conf_section_name section)
    {
        switch (section)
        {
            case conf_section_name::local:
                return "local";
            case conf_section_name::global:
                return "global";
            default:
                return "";
        }
    }

    enum class log_level
    {
        off,
        light,
        normal,
        hard
    };

    struct log_rotation
    {
        int value;
        char unit;
    };

    class conf_editor
    {
        private:
            std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_sections;
            std::unordered_map<std::string, std::string> m_global_section;
            std::unordered_map<std::string, std::string> m_local_section;
            std::string m_file;

            static void remove_quotes(std::string& str)
            {
                if (str.front() == '\'' && str.back() == '\'')
                    str = str.substr(1, str.size() - 2);
                else if (str.front() == '"' && str.back() == '"')
                    str = str.substr(1, str.size() - 2);
            }

            bool is_comment(const std::string& line)
            {
                return line[0] == '#';
            }

        public:
            conf_editor(const std::string& file = "")
            {
                if (!file.empty())
                    m_file = file;
                else
                    m_file = config::db_conf_file;
                parse();
            }

            std::unordered_map<std::string, std::string> get_section_contents(const std::string& section_name)
            {
                if (!section_name.empty())
                    return m_sections[section_name];
                throw core::conf_editor_exception(fmt::format("CONF_EDITOR error: section {} not found", section_name));
            }

            std::unordered_map<std::string, std::string> get_local_section()
            {
                return m_local_section;
            }

            std::unordered_map<std::string, std::string> get_global_section()
            {
                return m_global_section;
            }

            void parse()
            {
                std::ifstream file(m_file);
                if (!file.is_open())
                    throw core::conf_editor_exception("Could not open file: " + m_file);

                m_sections.clear();
                std::string current_section = "global";
                m_sections[current_section] = {};
                m_global_section = {};

                std::string line;

                while(std::getline(file, line))
                {
                    common::trim_str(line);

                    if (line.empty() || is_comment(line))
                        continue;

                    if (line.front() == '[' && line.back() == ']')
                    {
                        current_section = line.substr(1, line.size() - 2);
                        continue;
                    }

                    std::size_t delim_pos = line.find('=');
                    if (delim_pos != std::string::npos)
                    {
                        std::string key = line.substr(0, delim_pos);
                        std::string value = line.substr(delim_pos + 1);
                        
                        common::trim_str(key);
                        common::trim_str(value);
                        remove_quotes(value);
                        
                        if (current_section == "global")
                            m_global_section[key] = value;
                        else if (current_section == "local")
                            m_local_section[key] = value;

                        m_sections[current_section][key] = value;
                    }
                }
            }

            void save(const std::string& new_file_name = "")
            {
                std::string output_file = new_file_name.empty() ? m_file : new_file_name;
                std::ofstream file(output_file);
                if (!file.is_open())
                    throw core::conf_editor_exception("Could not open file for writing: " + output_file);

                file << "# Configuration of the local node\n";
                file << "# NOTE: Changing local section configuration overwrites identical settings defined in global section (only for this node).\n";
                file << "[local]\n\n";

                file << "# set default transaction isolation level (only for local transaction):\n";
                file << "# '1' - READ UNCOMMITED\n# '2' - READ COMMITED\n";
                file << "# '3' - REPEATABLE READ\n# '4' - SERIALIZABLE\n";
                file << "tx_isolation=" << (m_local_section.count("tx_isolation") ? m_local_section.at("tx_isolation") : m_global_section.at("tx_isolation")) << "\n\n";

                // Page encryption
                file << "# set page encryption\n# 'true'\n# 'false'\n";
                file << "page_encrypt=" << (m_local_section.count("page_encrypt") ? m_local_section.at("page_encrypt") : "false") << "\n\n";

                // Statistics calls
                file << "# Statistics calls in metadata manager before another refresh\n";
                file << "statistics_calls_limit=" << (m_local_section.count("statistics_calls_limit") ? m_local_section.at("statistics_calls_limit") : "100") << "\n\n";

                // Logs rotation
                file << "# Services and System logs rotation period:\n";
                file << "# 'Xm' - X months\n# 'Xd' - X days\n# 'Xh' - X hours\n";
                file << "sys_logs_rotation=" << (m_local_section.count("sys_logs_rotation") ? m_local_section.at("sys_logs_rotation") : m_global_section.at("sys_logs_rotation")) << "\n\n";

                // Logs level
                file << "# Services and System logging level:\n";
                file << "# 'off'\n# 'light'\n# 'normal'\n# 'hard'\n";
                file << "sys_logs_level='" << (m_local_section.count("sys_logs_level") ? m_local_section.at("sys_logs_level") : m_global_section.at("sys_logs_level")) << "'\n\n";

                // Global section
                file << "# Global configuration\n[global]\n\n";

                // Global logs rotation
                file << "# Services and System logs rotation period:\n";
                file << "# 'Xm' - X months\n# 'Xd' - X days\n# 'Xh' - X hours\n";
                file << "sys_logs_rotation=" << m_global_section.at("sys_logs_rotation") << "\n\n";

                // Global logs level
                file << "# Services and System logging level:\n";
                file << "# 'off'\n# 'light'\n# 'normal'\n# 'hard'\n";
                file << "sys_logs_level='" << m_global_section.at("sys_logs_level") << "'\n";
            }

            void set_isolation_level(tx::isolation_level isolation_level, conf_section_name section = conf_section_name::global)
            {
                auto level = std::to_string(static_cast<int>(isolation_level));
                if (section == conf_section_name::local)
                    m_local_section["tx_isolation"] = level;
                else if (section == conf_section_name::global)
                    m_global_section["tx_isolation"] = level;

                m_sections[section_name_to_string(section)]["tx_isolation"] = level;
            }

            void set_page_encryption(bool encryption_enabled, conf_section_name section = conf_section_name::global)
            {
                auto enabled = encryption_enabled ? "true" : "false";
                if (section == conf_section_name::local)
                    m_local_section["page_encrypt"] = enabled;
                else if (section == conf_section_name::global)
                    m_global_section["page_encrypt"] = enabled;

                m_sections[section_name_to_string(section)]["page_encrypt"] = enabled;
            }

            void set_statistics_calls_limit(int calls_limit, conf_section_name section = conf_section_name::global)
            {
                auto limit = std::to_string(calls_limit);
                if (section == conf_section_name::local)
                    m_local_section["statistics_calls_limit"] = limit;
                else if (section == conf_section_name::global)
                    m_global_section["statistics_calls_limit"] = limit;

                m_sections[section_name_to_string(section)]["statistics_calls_limit"] = limit; 
            }

            void set_logs_rotation(log_rotation logs_rotation, conf_section_name section = conf_section_name::global)
            {
                auto rotation = std::to_string(logs_rotation.value) + logs_rotation.unit;
                if (section == conf_section_name::local)
                    m_local_section["sys_logs_rotation"] = rotation;
                else if (section == conf_section_name::global)
                    m_global_section["sys_logs_rotation"] = rotation;

                m_sections[section_name_to_string(section)]["sys_logs_rotation"] = rotation; 
            }

            void set_logging_level(log_level logging_level, conf_section_name section = conf_section_name::global)
            {
                std::string level;
                switch (logging_level)
                {
                    case log_level::off:
                        level = "OFF";
                        break;
                    case log_level::light:
                        level = "LIGHT";
                        break;
                    case log_level::normal:
                        level = "NORMAL";
                        break;
                    case log_level::hard:
                        level = "HARD";
                        break;
                }
                if (section == conf_section_name::local)
                    m_local_section["sys_logs_level"] = level;
                else if (section == conf_section_name::global)
                    m_global_section["sys_logs_level"] = level;

                m_sections[section_name_to_string(section)]["sys_logs_level"] = level;
            }
    };
}

#endif
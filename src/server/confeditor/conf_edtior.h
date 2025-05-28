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

namespace server::conf
{
    class conf_editor
    {
        private:
            std::unordered_map<std::string, std::unordered_map<std::string, std::string>> m_sections;
            std::string m_current_section;
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

            void parse()
            {
                std::ifstream file(m_file);
                if (!file.is_open())
                    throw std::runtime_error("Could not open file: " + m_file);

                m_sections.clear();
                m_current_section = "global";
                m_sections[m_current_section] = {};

                std::string line;

                while(std::getline(file, line))
                {
                    common::trim_str(line);

                    if (line.empty() || is_comment(line))
                        continue;

                    if (line.front() == '[' && line.back() == ']')
                    {
                        m_current_section = line.substr(1, line.size() - 2);
                        m_sections[m_current_section];
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
                        
                        m_sections[m_current_section][key] = value;
                    }
                }
            }

            void save()
            {
                
            }
    };
}

#endif
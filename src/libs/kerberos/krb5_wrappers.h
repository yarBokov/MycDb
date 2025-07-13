#ifndef __KERBEROS_KRB5_WRAPPERS_H
#define __KERBEROS_KRB5_WRAPPERS_H

#include <k5-int.h>
#include <kadm5/admin.h>
#include <krb5/krb5.h>
#include <type_traits>
#include <memory>
#include <cstring>

namespace kerberos
{
    struct type_base
    {
        explicit type_base(krb5_context ctx) : context(ctx) {}
    
        type_base(const type_base&) = delete;
        type_base& operator=(const type_base&) = delete;
        type_base(type_base&&) = delete;
        type_base& operator=(type_base&&) = delete;

        template<typename T>
        void deallocate(T object) noexcept
        {
            if (!context) return;

            if constexpr (std::is_same_v<T, krb5_auth_context>)
                ::krb5_auth_con_free(context, object);
            else if constexpr (std::is_same_v<T, krb5_principal>)
                ::krb5_free_principal(context, object);
            else if constexpr (std::is_same_v<T, krb5_keytab>)
                ::krb5_kt_close(context, object);
            else if constexpr (std::is_same_v<T, krb5_ticket*>)
                ::krb5_free_ticket(context, object);
            else if constexpr (std::is_same_v<T, krb5_creds*>)
                ::krb5_free_cred_contents(context, object);
            else if constexpr (std::is_same_v<T, krb5_error*>)
                ::krb5_free_error(context, object);
            else if constexpr (std::is_same_v<T, krb5_ap_rep_enc_part*>)
                ::krb5_free_ap_rep_enc_part(context, object);
            else if constexpr (std::is_same_v<T, krb5_ccache>)
                ::krb5_cc_destroy(context, object);
            else if constexpr (std::is_same_v<T, krb5_get_init_creds_opt*>)
                ::krb5_get_init_creds_opt_free(context, object);
            else if constexpr (std::is_same_v<T, krb5_keyblock*>)
                ::krb5_free_keyblock(context, object);
            else if constexpr (std::is_same_v<T, krb5_data*>)
                ::krb5_free_data_contents(context, object);
            else
                static_assert(!std::is_same_v<T, T>, "Unsupported type for deallocation");
        }

        krb5_context context{};
    };

    template<typename T>
    struct wrapper_type final : public type_base
    {
        using value_type = T;

        explicit wrapper_type(krb5_context ctx) : type_base(ctx)
        {
            std::memset(&obj_val, 0, sizeof(T));
        }

        ~wrapper_type()
        {
            deallocate(&obj_val);
        }

        value_type* oeprator&() noexcept { return &obj_val; }
        const value_type* oeprator&() const noexcept { return &obj_val; }

        value_type obj_val{};
    };

    template<>
    struct wrapper_type<kadm5_key_data> final : public type_base
    {
        using value_type = kadm5_key_data*;

        explicit wrapper_type(krb5_context ctx) : type_base(ctx) {}
        ~wrapper_type() { if (pointer) kadm5_free_kadm5_key_data(context, keys, pointer); }

        void set_keys(int count) noexcept { keys = count; }

        value_type* operator&() noexcept { return &pointer; }
        const value_type* operator&() const noexcept { return &pointer; }
        explicit operator value_type() const noexcept { return pointer; }

        int keys{};
        value_type pointer{};
    };

    using principal_wrapper = wrapper_type<krb5_principal>;
    using init_creds_opt_wrapper = wrapper_type<krb5_get_init_creds_opt*>;
    using creds_wrapper = wrapper_type<krb5_creds>;
}

#endif
#include <erl_nif.h>
#include <dkim-regdom.h>

#include <ctype.h>
#include <string.h>

extern char *tldString;

static ERL_NIF_TERM
make_atom(ErlNifEnv *env, const char *name)
{
    ERL_NIF_TERM ret;

    if (enif_make_existing_atom(env, name, &ret, ERL_NIF_LATIN1)) {
        return ret;
    }
    return enif_make_atom(env, name);
}

static ERL_NIF_TERM
make_error(ErlNifEnv *env, const char *mesg)
{
    ERL_NIF_TERM error = make_atom(env, "error");
    return enif_make_tuple2(env, error, make_atom(env, mesg));
}

static ERL_NIF_TERM
make_binary_string(ErlNifEnv *env, const char *s)
{
    ErlNifBinary b;
    int n;

    n = strlen(s);
    enif_alloc_binary(n, &b);
    memcpy(b.data, s, n);
    b.size = n;
    return enif_make_binary(env, &b);
}

static ERL_NIF_TERM
get_registered_domain(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary input;
    ERL_NIF_TERM retval;

    tldnode *root = (tldnode*) enif_priv_data(env);

    char *result;
    char *signing_domain;

    if (argc != 1) {
        return enif_make_badarg(env);
    }
    if (!enif_inspect_iolist_as_binary(env, argv[0], &input)) {
        return enif_make_badarg(env);
    }

    signing_domain = (char *) enif_alloc(input.size+1);
    if (!signing_domain)
        return make_error(env, "out_of_memory");
    memcpy(signing_domain, input.data, input.size);
    signing_domain[input.size] = '\0';

    result = getRegisteredDomain(signing_domain, root);

    enif_free(signing_domain);

    if (result == NULL)
	return make_error(env, "no_registered_domain");

    retval = make_binary_string(env, result);

    free(result);

    return retval;
}

static int
on_load(ErlNifEnv *env, void **priv, ERL_NIF_TERM info)
{

    tldnode *root = readTldTree(tldString);

    *priv = (void *) root;

    return 0;
}

static int
on_upgrade(ErlNifEnv *env, void **new_priv, void **old_priv, ERL_NIF_TERM info)
{
    return on_load(env, new_priv, info);
}


static void
on_unload(ErlNifEnv *env, void *priv)
{
    freeTldTree((tldnode*)priv);
}


static ErlNifFunc nif_functions[] = {
    {"get_registered_domain", 1, get_registered_domain}
};

ERL_NIF_INIT(erl_effective_tld, nif_functions, &on_load, NULL, &on_upgrade, &on_unload);

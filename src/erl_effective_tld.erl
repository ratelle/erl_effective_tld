-module(erl_effective_tld).

-export([
    init/0,
    get_registered_domain/1
]).

-on_load(init/0).

init() ->
    PrivDir = case code:priv_dir(?MODULE) of
                  {error, _} ->
                      EbinDir = filename:dirname(code:which(?MODULE)),
                      AppPath = filename:dirname(EbinDir),
                      filename:join(AppPath, "priv");
                  Dir -> Dir
              end,
    SoName = filename:join(PrivDir, "erl_effective_tld_nif"),
    case catch erlang:load_nif(SoName, 0) of
        ok -> ok;
        LoadError -> error_logger:error_msg("erl_effective_tld: error loading NIF (~p): ~p",
                                            [SoName, LoadError])
    end.

get_registered_domain(signing_domain) ->
    {error, erl_effective_tld_nif_not_loaded}.

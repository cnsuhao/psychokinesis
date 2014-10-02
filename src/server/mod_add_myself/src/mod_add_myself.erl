%%%----------------------------------------------------------------------
%%% File    : mod_add_myself.erl
%%% Author  : wuyingfengsui <wuyingfengsui@gmail.com>
%%% Purpose : make roster with myself after registered
%%% Created : 19 July 2014 by wuyingfengsui <wuyingfengsui@gmail.com>
%%% Updated : 
%%% Notes   : depends mod_restful
%%%----------------------------------------------------------------------

-module(mod_add_myself).
-author('wuyingfengsui@gmail.com').

-behaviour(gen_mod).
-behaviour(gen_restful_api).

-export([start/2,
		 stop/1,
		 
		 % mod_restful_register events
		 mod_restful_register_registered/4,
		 
		 % mod_restful API
         process_rest/1
		]).
		 
-define(LAGER, 1).

-include("ejabberd.hrl").
-include("logger.hrl").        %% INFO_MSG需要
-include("jlib.hrl").


start(_Host, _Opts) ->
	?INFO_MSG("mod_add_myself started ~p~n", [_Opts]),
	ejabberd_hooks:add(mod_restful_register_registered, _Host, ?MODULE, mod_restful_register_registered, 50).
	
stop(_Host) ->
    ?INFO_MSG("mod_add_myself stopped~n", []),
	ejabberd_hooks:delete(mod_restful_register_registered, _Host, ?MODULE, mod_restful_register_registered, 50).
	
mod_restful_register_registered(AccIn, Username, Host, Request) ->
	mod_admin_extra:add_rosteritem(Username, Host, Username, Host, Username, list_to_binary("my"), list_to_binary("both")),
	AccIn.
	
process_rest(Request) -> {simple, ok}.
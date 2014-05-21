%%%----------------------------------------------------------------------
%%% File    : mod_filter.erl
%%% Author  : wuyingfengsui <wuyingfengsui@gmail.com>
%%% Purpose : filter message for psychokinesis
%%% Created : 17 May 2014 by wuyingfengsui <wuyingfengsui@gmail.com>
%%% Updated : 21 May 2014 by wuyingfengsui <wuyingfengsui@gmail.com>
%%%----------------------------------------------------------------------

-module(mod_filter).
-author('wuyingfengsui@gmail.com').

-behaviour(gen_mod).

-export([start/2,
		 stop/1,
		 filter_packet/1]).
		 
-define(LAGER, 1).

-include("ejabberd.hrl").
-include("logger.hrl").
-include("jlib.hrl").

-define(PROCNAME, ?MODULE).
-define(DEFAULT_PATH, ".").
-define(DEFAULT_FORMAT, text).

start(_Host, _Opts) ->
	?INFO_MSG("mod_filter started ~p~n", [_Opts]),
	ejabberd_hooks:add(filter_packet, global, ?MODULE, filter_packet, 0).
	
stop(_Host) ->
    ?INFO_MSG("mod_filter stopped~n", []),
	ejabberd_hooks:delete(filter_packet, global, ?MODULE, filter_packet, 0).
	
filter_packet({From, To, Packet} = Input) ->
	%% return modified Packet or atom `drop` to drop the packet
	if To#jid.luser /= <<>>,
	   From#jid.luser /= <<>>,
	   From#jid.luser /= To#jid.luser ->
			?INFO_MSG("drop the message:~n from:~p~n  to:~p~n packet:~p~n", [From, To, Packet]),
			drop;
	   true ->
			Input
	end.
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">

<head>
<title>{$lang.live_help}</title>
<link rel="stylesheet" type="text/css" href="{$config.skin_path}/styles.css" />
<link rel="stylesheet" type="text/css" href="{$config.skin_path}/addons/live_help/styles.css" />

<!--[if IE]>
<link href="{$config.skin_path}/styles_ie.css" rel="stylesheet" type="text/css" />
<![endif]-->
<meta http-equiv="Content-Type" content="text/html; charset={$smarty.const.CHARSET}" />

{include file="common_templates/scripts.tpl"}

{script src="addons/live_help/js/live_help.js"}
{script src="addons/live_help/js/chat.js"}

</head>

<body style="min-width: 90%;">

<div id="enter" align="center" style="padding-top: 130px;{if $session_data}display: none;{/if}">
	{capture name="sidebox"}
		<form name="enter_chat_form" onsubmit="live_help_chat.enter_chat(); return false;">
		<p>{$lang.lh_chat_login_intro}</p>
		<p><input style="width:90%" type="text" id="auth_name" value="{$lang.chat_default_name|escape:html}" maxlength="255" /></p>
		<p align="center">{include file="buttons/button.tpl" but_id="enter_chat_submit" but_text=$lang.lh_enter_chat but_role="action" but_onclick="live_help_chat.enter_chat();"}</p>
		</form>
	{/capture}
	{include file="blocks/wrappers/sidebox_general.tpl" width="250" title=$lang.lh_chat_login_caption content=$smarty.capture.sidebox}
</div>

<div id="chat"{if !$session_data} class="hidden"{/if}>

	{*
	<div class="border" style="padding: 1px;">
		<span id="type_notify" class="ajax-message" style="position: absolute; top: 9px; right: 9px; display: none; padding: 5px; margin: 0px;">{$lang.lh_type_notify}</span>
		<div class="mainbox-body" style="padding: 5px; height: 350px; overflow: auto;" id="messages_log"></div>
	</div>
	*}
	{if $invitation_messages}
	{/if}
	<div class="border" style="padding: 1px;">
		<span id="type_notify" class="ajax-message" style="position: absolute; top: 9px; right: 9px; display: none; padding: 5px; margin: 0px;">{$lang.lh_type_notify}</span>
		<div class="mainbox-body" style="padding: 5px; height: 350px; overflow: auto;" id="messages"></div>
	</div>

	<table cellpadding="0" cellspacing="0" width="100%" border="0">
	<tr>
		<td width="100%">
			<input type="text" id="message_text" onkeyup="live_help_chat.on_typing(event, this.value);" class="input-text-100" maxlength="1000" />
		</td>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>
		<td nowrap="nowrap">
			{include file="buttons/button.tpl" but_id="send_button" but_text=$lang.lh_send_button but_onclick="live_help_chat.send_message();" but_role="text"}
		</td>
		<td>&nbsp;&nbsp;&nbsp;&nbsp;</td>
	</tr>
	</table>

</div>


<script type="text/javascript">
//<![CDATA[
lang['chat_operator_joined'] = '{$lang.lh_chat_operator_joined|escape:javascript}';
lang['chat_operator_left'] = '{$lang.lh_chat_operator_left|escape:javascript}';
lang['chat_recieved_file'] = '{$lang.lh_chat_recieved_file|escape:javascript}';
lang['chat_opened_link'] = '{$lang.lh_chat_opened_link|escape:javascript}';
lang['chat_please_wait'] = '{$lang.lh_chat_please_wait|escape:javascript}';
lang['chat_session_started'] = '{$lang.lh_chat_session_started|escape:javascript}';
lang['please_input_name'] = '{$lang.lh_please_input_name|escape:javascript}';
lang['info_unknown'] = '{$lang.lh_info_unknown|escape:javascript}';
lang['operator_wait'] = '{$lang.lh_chat_please_wait|addslashes}';

params = {$ldelim}
	{if $session_data}
	session_data: {$ldelim}
	{foreach from=$session_data item=value key=param name=fef}
		{$param}: '{$value}'{if !$smarty.foreach.fef.last},{/if}
	{/foreach}
	{$rdelim},
	{/if}
	request_url: '{$index_script}?',
	request_interval: {$request_interval},
	visitor_chat_name: '{$visitor_chat_name}'
{$rdelim}

invitation_m = {$ldelim}
	{if $invitation_messages}
	{foreach from=$invitation_messages item=value key=param name=fef}
		{$param}: '{$value}'{if !$smarty.foreach.fef.last},{/if}
	{/foreach}
	{/if}
{$rdelim}
$( function(){$ldelim}
	live_help_chat.init(params, invitation_m);
{$rdelim});
//]]>
</script>
</body>
</html>
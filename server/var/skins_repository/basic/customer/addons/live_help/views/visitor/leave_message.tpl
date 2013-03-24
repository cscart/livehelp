<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">

<head>
<title>{$lang.live_help}</title>
{include file="common_templates/styles.tpl"}
<link href="{$config.skin_path}/addons/live_help/styles.css" rel="stylesheet" type="text/css" />
{include file="common_templates/scripts.tpl"}
</head>

<body>

{capture name="content"}

<div>{if $action == "sent"}{$lang.lh_leave_message_sent}{else}{$lang.lh_leave_message_intro}{/if}</div>

{if $action == "sent"}
	<p>{include file="buttons/button.tpl" but_text=$lang.close but_onclick="window.close();"}</p>
{else}
<form action="{"visitor.leave_message"|fn_url}" method="post" name="leave_message_form">

<div class="form-field">
	<label for="elm_name" class="cm-required">{$lang.name}:</label>
	<input id="elm_name" type="text" name="message_data[from_name]" maxlenght="255" size="50" class="input-text" />
</div>

<div class="form-field">
	<label for="elm_mail" class="cm-required cm-email">{$lang.email}:</label>
	<input id="elm_mail" type="text" name="message_data[from_mail]" maxlenght="255" size="50" class="input-text" />
</div>

<div class="form-field">
	<label for="elm_subject" class="cm-required">{$lang.subject}:</label>
	<input id="elm_subject" type="text" name="message_data[subject]" maxlenght="255" size="50" class="input-text" />
</div>

<div class="form-field">
	<label for="elm_message" class="cm-required">{$lang.message}:</label>
	<textarea id="elm_message" name="message_data[message]" cols="72" rows="10" class="input-textarea"></textarea></p>
</div>

<div class="buttons-container">
{include file="buttons/button.tpl" but_text=$lang.lh_leave_message_submit but_role="action" but_name="dispatch[lh_visitor.leave_message]"}
&nbsp;{include file="buttons/button.tpl" but_text=$lang.close but_onclick="window.close();"}
</div>

</form>
{/if}

{/capture}
{include file="blocks/wrappers/mainbox_general.tpl" content=$smarty.capture.content title=$lang.lh_leave_message_submit}
</body>

</html>
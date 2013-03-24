<div id="invitation_dialog_bg" class="ajax-message-wrapper" style="z-index:10000;">
	<div class="ajax-message">
		<p><strong id="invitation_text"></strong></p>
		<p>{$lang.lh_invitation_instruction}</p>
		<form action="javascript: live_help.accept_invitation(); void(0);" name="accept_invitation_form" onsubmit="live_help_chat.accept_invitation(); return false;">
		<input type="hidden" id="op_id" name="operator_id" value="" />
		<input id="invitation_name" type="text" class="input-text" value="{$lang.chat_default_name}" maxlength="255"/>
		<div class="buttons-container">
			{include file="buttons/button.tpl" but_text=$lang.accept but_onclick="live_help.accept_invitation();" but_role="action"}
			&nbsp;{include file="buttons/button.tpl" but_text=$lang.decline but_onclick="live_help.decline_invitation();"}
		</div>
		</form>
	</div>
</div>
{capture name="mainbox"}

<form action="{""|fn_url}" method="post" class="cm-form-highlight" name="operators_update_form">
<input type="hidden" name="operator_data[operator_id]" value="{$operator_data.operator_id}" />

<fieldset>

<div class="form-field">
	<label for="elm_username" class="cm-required">{$lang.username}:</label>
	<input type="text" id="elm_username" name="operator_data[login]" size="32" value="{$operator_data.login}" class="input-text-large main-inputs" />
</div>

<div class="form-field">
	<label for="elm_name" class="cm-required">{$lang.name}:</label>
	<input type="text" id="elm_name" name="operator_data[name]" size="32" value="{$operator_data.name}" class="input-text-large" />
</div>

<div class="form-field">
	<label for="elm_password1" class="cm-required">{$lang.password}:</label>
	<input type="password" id="elm_password1"  name="operator_data[password1]" size="32" value="{$operator_data.password1}" class="input-text-medium" />
</div>

<div class="form-field">
	<label for="elm_password2" class="cm-required">{$lang.confirm_password}:</label>
	<input type="password" id="elm_password2"  name="operator_data[password2]" size="32" value="{$operator_data.password2}" class="input-text-medium" />
</div>

{include file="common_templates/select_status.tpl" input_name="operator_data[status]" id="operator_data" obj=$operator_data}

</fieldset>

<div class="buttons-container buttons-bg">
	{if $mode == "add"}
		{include file="buttons/save_cancel.tpl" but_name="dispatch[operators.add]"}
	{else}
		{include file="buttons/save_cancel.tpl" but_name="dispatch[operators.update]"}
	{/if}
</div>

</form>
{/capture}

{if $mode == "add"}
	{assign var="_title" value="`$lang.new_operator`"}
{else}
	{assign var="_title" value="`$lang.editing_operator`:&nbsp;`$operator_data.name`"}
{/if}
	{include file="common_templates/mainbox.tpl" title=$_title content=$smarty.capture.mainbox}
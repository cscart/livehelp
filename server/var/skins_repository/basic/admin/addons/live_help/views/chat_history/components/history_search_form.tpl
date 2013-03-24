{capture name="section"}

<form action="{""|fn_url}" name="search_form" method="get">
<input type="hidden" name="page" value="{$search.page}" />

<table cellpadding="0" cellspacing="0" border="0" class="search-header">
<tr>
	<td class="nowrap search-field">
		<label for="name">{$lang.name}:</label>
		<div class="break">
			<input type="text" name="name" id="name" size="30" value="{$search.name}" class="input-text" />
		</div>

	</td>

	<td class="nowrap search-field">
		<label for="match">{$lang.lh_message_contains_word}:</label>
		<div class="break">
			<select name="match" id="match">
				<option value="any" {if $search.match == "any" || !$search.match}selected="selected"{/if}>{$lang.any_words}</option>
				<option value="all" {if $search.match == "all"}selected="selected"{/if}>{$lang.all_words}</option>
				<option value="exact" {if $search.match == "exact"}selected="selected"{/if}>{$lang.exact_phrase}</option>
			</select>
			&nbsp;<input type="text" name="q" size="45" value="{$search.q}" class="input-text" />
		</div>
	</td>

	<td class="buttons-container">
		{include file="buttons/button.tpl" but_text=$lang.search but_name="dispatch[chat_history.manage]" but_role="submit"}
	</td>
</tr>
</table>

{capture name="advanced_search"}

<div class="search-field">
	<label for="operator">{$lang.lh_filter_operators}:</label>
	<select name="id" id="operator">
		<option value="">--</option>
		{foreach from=$operators item=operator}
			<option value="{$operator.id}" {if $search.id == $operator.id}selected="selected"{/if}>{$operator.login} ({$operator.name})</option>
		{/foreach}
	</select>
</div>

<div class="search-field">
	<label for="group">{$lang.lh_history_group}:</label>
	<select name="group" id="group">
		<option value="">--</option>
		<option value="chat" {if $search.group == "chat"}selected="selected"{/if}>{$lang.lh_chat_sessions}</option>
		<option value="message" {if $search.group == "message"}selected="selected"{/if}>{$lang.lh_left_messages}</option>
		<option value="operator" {if $search.group == "operator"}selected="selected"{/if}>{$lang.lh_operators_chat}</option>
	</select>
</div>

<div class="search-field">
	<label for="email">{$lang.email}:</label>
	<input type="text" name="email" id="email" size="30" value="{$search.email}" class="input-text" />
</div>

<div class="search-field">
	<label>{$lang.period}:</label>
	{include file="common_templates/period_selector.tpl" period=$smarty.request.period form_name="search_form"}
</div>

{/capture}

{include file="common_templates/advanced_search.tpl" content=$smarty.capture.advanced_search dispatch="chat_history.manage" view_type="chat_history"}

</form>

{/capture}
{include file="common_templates/section.tpl" section_title=$lang.site_visitors_filter section_content=$smarty.capture.section}
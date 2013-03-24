{capture name="mainbox"}

{include file="addons/live_help/views/chat_history/components/history_search_form.tpl"}

{include file="common_templates/pagination.tpl"}

{assign var="c_url" value=$config.current_url|fn_query_remove:"sort_by":"sort_order"}

<table cellpadding="0" cellspacing="0" border="0" width="100%" class="table sortable">
<tr>
	<th><a class="cm-ajax{if $sort_by == "type"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=type&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.group}</a></th>
	<th width="40%"><a class="cm-ajax{if $sort_by == "name"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=name&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.name}</a></th>
	<th><a class="cm-ajax{if $sort_by == "operator_name"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=operator_name&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.lh_operator}</a></th>
	<th><a class="cm-ajax{if $sort_by == "num"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=num&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.messages}</a></th>
	<th><a class="cm-ajax{if $sort_by == "min_date"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=min_date&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.lh_earliest}</a></th>
	<th><a class="cm-ajax{if $sort_by == "max_date"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=max_date&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.lh_oldest}</a></th>
	{hook name="live_help:header"}
	{/hook}
</tr>
{foreach from=$persons item=person}
	{if $person.type == "chat"}
		<tr class="text-group-chat">
	{elseif $person.type == "message"}
		<tr class="text-group-message">
	{elseif $person.type == "operator"}
		<tr class="text-group-operator">
	{/if}
		<td>{if $person.type == "chat"}{$lang.lh_chat_session}{elseif $person.type == "message"}{$lang.lh_left_message}{elseif $person.type == "operator"}{$lang.lh_operator_chat}{/if}</td>
		<td>
			{capture name="_href"}chat_history.view?type={$person.type}&amp;id={$person.id}&amp;operator={$person.operator_id}{if $params.q}&amp;highlight={$params.q}{/if}&amp;reset_period=1{if $person.selected_section}&amp;selected_section={$person.selected_section}{/if}{/capture}
			<a href="{$smarty.capture._href|fn_url}">{$person.name}</a>
		</td>
		<td>{$person.operator_name}</td>
		<td>{$person.num}</td>
		<td>{$person.min_date|date_format:"`$settings.Appearance.date_format`, `$settings.Appearance.time_format`"}</td>
		<td>{$person.max_date|date_format:"`$settings.Appearance.date_format`, `$settings.Appearance.time_format`"}</td>
		{hook name="live_help:content"}
		{/hook}
	</tr>
	{if $params.q}
		{foreach from=$texts item=row}
				{if $person.type == "chat" && ($row.operator_name == $person.operator_name && $row.name == $person.name)}
				<tr>
					<td colspan="7" class="text-group-chat">-&nbsp;&nbsp;{$row.text}</td>
				</tr>
				{elseif $person.type == "message" && ($row.operator_name == $person.operator_name && $row.name == $person.name)}
				<tr>
					<td colspan="7" class="text-group-message">-&nbsp;&nbsp;{$row.text}</td>
				</tr>
				{elseif $person.type == "operator" && (($row.operator_name == $person.operator_name && $row.name == $person.name) ||	($row.operator_name == $person.name && $row.name == $person.operator_name))}
				<tr>
					<td colspan="7" class="text-group-operator">-&nbsp;&nbsp;{$row.text}</td>
				</tr>
				{/if}
		{/foreach}
	{/if}
{foreachelse}
<tr class="no-items">
	<td colspan="7"><p>{$lang.no_data}</p></td>
</tr>
{/foreach}
</table>


{include file="common_templates/pagination.tpl"}

{/capture}

{include file="common_templates/mainbox.tpl" title=$lang.lh_chat_history content=$smarty.capture.mainbox title_extra=$smarty.capture.title_extra}
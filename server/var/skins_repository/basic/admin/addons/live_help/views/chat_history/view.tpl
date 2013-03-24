<p align="right">
	<span class="bull">&bull;</span>&nbsp;<a href="javascript: if (confirm('{$lang.delete_livehelp_warning|escape:javascript}')) $.redirect('{"chat_history.delete"|fn_url}'); else void(0);" class="underlined">{$lang.remove_livehelp_data}</a>
</p>

{capture name="mainbox"}

{if $type != "message"}
{capture name="section"}
<form action="{""|fn_url}" method="get" name="report_form_visitor">
<input type="hidden" name="type" value="{$type}" />
<input type="hidden" name="id" value="{$id}" />
<input type="hidden" name="operator" value="{$operator}" />
<input type="hidden" name="selected_section" value="{$selected_section}" />
<input type="hidden" name="action" value="update" />
{include file="common_templates/period_selector.tpl" period=$smarty.request.period form_name="report_form_visitor" display="form" but_name="dispatch[chat_history.view]"}
</form>
{/capture}
{include file="common_templates/section.tpl" section_content=$smarty.capture.section}
{/if}

{capture name="tabsbox"}

{include file="common_templates/pagination.tpl"}

{if $selected_section == "events_log" || $selected_section == "notes"}

	{assign var="c_url" value=$config.current_url|fn_query_remove:"sort_by":"sort_order"}

	<table cellpadding="0" cellspacing="0" border="0" width="100%" class="table sortable">
	<tr>
		{foreach from=$columns item=column}
			{assign var="column_lang" value=$column.lang}
			<th {if $column.type == "date" || $column.type == "operator"}width="15%"{/if}><a class="cm-ajax{if $sort_by == $column.name} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=`$column.name`&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">
				{$lang.$column_lang}
			</a></th>
		{/foreach}
	</tr>
	{foreach from=$data item=item}
	<tr {cycle values="class=\"table-row\", "}>
		{assign var="column_num" value=1}
		{foreach from=$columns item=column}
			{assign var="value" value=$item[$column.name]}

			{if $column.type == "date"}
				<td>{$value|date_format:"`$settings.Appearance.date_format`, `$settings.Appearance.time_format`"}</td>
				
			{elseif $column.type == "event"}
				{assign var="event_lang" value=$event_to_lang[$value]}
				<td>{$lang.$event_lang|replace:"[name]":"<span>`$item.operator`</span>"}</td>
				
			{elseif $column.type == "url"}
				<td><a href="{$value|fn_url}" title="{$value}" target="_blank">{$value|replace:"&amp;":"&"|fn_compact_value:50}</a></td>
				
			{else}
				<td>{$value}</td>
			{/if}
			{math equation="column_num+1" column_num=$column_num assign="column_num"}
		{/foreach}
	</tr>
	{foreachelse}
	<tr class="no-items">
		<td colspan="{$column_num}"><p>{$lang.no_items}</p></td>
	</tr>
	{/foreach}
	</table>
	
	{if $data}
		{include file="common_templates/table_tools.tpl" href="#history_view"}
	{/if}
{else}

	<table cellpadding="3" cellspacing="1" border="0" width="100%">
	{foreach from=$messages item=message}
	<tr {cycle values="class=\"manage-row\", "}>
		<td>
			<img src="{$images_dir}/icons/chat_arrow_{if $message.direction == 0}in{else}out{/if}.gif" width="4" height="7" alt="" />
			<span class="{if $message.direction == 0}chat-visitor{else}chat-operator{/if}">
				&nbsp;[{$message.date|date_format:"`$settings.Appearance.date_format`, `$settings.Appearance.time_format`"}]
				&lt;{$message.from_name}&gt;</span>&nbsp;
			{$message.message}</td>
	</tr>
	{foreachelse}
	<tr class="no-items">
		<td><p>{$lang.no_items}</p></td>
	</tr>
	{/foreach}
	</table>

{/if}

{include file="common_templates/pagination.tpl"}

{/capture}
{include file="common_templates/tabsbox.tpl" content=$smarty.capture.tabsbox active_tab=$selected_section}

{/capture}
{include file="common_templates/mainbox.tpl" title=$lang.lh_chat_history content=$smarty.capture.mainbox}

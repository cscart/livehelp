{capture name="mainbox"}

<form action="{""|fn_url}" method="post" name="manage_operators_form" >
{include file="common_templates/pagination.tpl"}

{assign var="c_url" value=$config.current_url|fn_query_remove:"sort_by":"sort_order"}

<table cellpadding="0" cellspacing="0" border="0" width="100%" class="table sortable">
<tr>
	<th class="center" width="1%"><input type="checkbox" name="check_all" value="Y" title="{$lang.check_uncheck_all}" class="checkbox cm-check-items" /></th>
	<th><a class="cm-ajax{if $sort_by == "operator_id"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=operator_id&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.id}</a></th>
	<th><a class="cm-ajax{if $sort_by == "login"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=login&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.username}</a></th>
	<th><a class="cm-ajax{if $sort_by == "name"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=name&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.name}</a></th>
	<th><a class="cm-ajax{if $sort_by == "register_time"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=register_time&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.registered}</a></th>
	<th width="15%"><a class="cm-ajax{if $sort_by == "status"} sort-link-{$sort_order}{/if}" href="{"`$c_url`&amp;sort_by=status&amp;sort_order=`$sort_order`"|fn_url}" rev="pagination_contents">{$lang.status}</a></th>
	<th>&nbsp;</th>
</tr>
{foreach from=$operators item=operator}
<tr {cycle values="class=\"table-row\", "}>
	<td class="center" width="1%"><input type="checkbox" name="operator_ids[]" value="{$operator.operator_id}" class="checkbox cm-item" /></td>
	<td><a href="{"operators.update?operator_id=`$operator.operator_id`"|fn_url}">{$operator.operator_id}</a></td>
	<td><a href="{"operators.update?operator_id=`$operator.operator_id`"|fn_url}">{$operator.login}</a></td>
	<td><a href="{"operators.update?operator_id=`$operator.operator_id`"|fn_url}">{$operator.name}</a></td>
	<td>{$operator.register_time|date_format:"`$settings.Appearance.date_format`, `$settings.Appearance.time_format`"}</td>
	<td>
		{include file="common_templates/select_popup.tpl" id=$operator.operator_id status=$operator.status hidden="" object_id_name="operator_id" table="lh_operators"}
	</td>
	<td>
		{include file="buttons/button.tpl" but_href="operators.update?operator_id=`$operator.operator_id`" but_text=$lang.edit but_role="edit"}
	</td>
</tr>
{foreachelse}
<tr class="no-items">
	<td colspan="6"><p>{$lang.no_items}</p></td>
</tr>
{/foreach}
</table>

{if $operators}
	{include file="common_templates/table_tools.tpl" href="#operators" visibility="Y"}
{/if}

{include file="common_templates/pagination.tpl"}

<div class="buttons-container buttons-bg">
	{if $operators}
	<div class="float-left">
		{capture name="tools_list"}
		<ul>
			<li><a name="dispatch[operators.delete]" class="cm-process-items cm-confirm" rev="manage_operators_form">{$lang.delete_selected}</a></li>
		</ul>
		{/capture}
		{include file="buttons/save.tpl" but_name="dispatch[operators.m_update]" but_role="button_main"}
		{include file="common_templates/tools.tpl" prefix="main" hide_actions=true tools_list=$smarty.capture.tools_list display="inline" link_text=$lang.choose_action}
	</div>
	{/if}
	
	<div class="float-right">
		{include file="common_templates/tools.tpl" tool_href="operators.add" prefix="bottom" hide_tools="true" link_text=$lang.add_operator}
	</div>
</div>

{capture name="tools"}
	{include file="common_templates/tools.tpl" tool_href="operators.add" prefix="top" hide_tools="true" link_text=$lang.add_operator}
{/capture}

</form>

{/capture}
{include file="common_templates/mainbox.tpl" title=$lang.lh_operators content=$smarty.capture.mainbox tools=$smarty.capture.tools}
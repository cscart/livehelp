{** block-description:live_help **}

<!--dynamic:live_help-->
{script src="addons/live_help/js/live_help.js"}
{script src="addons/live_help/js/visitor.js"}

<script type="text/javascript">
//<![CDATA[
	lang['chatbox_title'] = '{$lang.live_help|escape:javascript}';
	lang['failed_popup'] = '{$lang.lh_failed_popup|escape:javascript}';
	lang['not_available'] = '{$lang.lh_sidebox_not_available|escape:javascript}';
	lang['image_offline_hint'] = '{$lang.lh_status_image_offline_hint|escape:javascript}';
	lang['image_online_hint'] = '{$lang.lh_status_image_online_hint|escape:javascript}';
	lang['please_input_name'] = '{$lang.lh_please_input_name|escape:javascript}';
	lang['info_unknown'] = '{$lang.lh_info_unknown|escape:javascript}';

	params = {$ldelim}
		request_url: '{$index_script}?dispatch=',
		status_image_path: '{$images_dir}/',
		allow_leave_message: '{$live_help_opts.lh_allow_leave_message}'
	{$rdelim}

	$( function() {$ldelim}
		live_help_visitor.init(params);
	{$rdelim});
//]]>
</script>

{include file="addons/live_help/views/visitor/components/invitation.tpl"}

{if $live_help_opts.lh_show_status_image == "Y" || $live_help_opts.lh_show_operators_num == "Y" || $live_help_opts.lh_allow_start_chat == "Y" || $live_help_opts.lh_allow_leave_message == "Y"}
	{if $live_help_opts.lh_show_status_image == "Y"}
		<div align="center"><div style="width: 113px; height: 45px;">
			<a id="lh_status_image_link">
				<img id="lh_status_image" src="" style="display:none; border:0px" alt="" />
			</a>
		</div></div>
	{/if}
	{if $live_help_opts.lh_show_operators_num == "Y"}
		<div>{$lang.lh_sidebox_total_operators} <strong id="lh_total_count"></strong></div>
		<div>{$lang.lh_sidebox_free_operators} <strong id="lh_free_count"></strong></div>
	{/if}

	{if $live_help_opts.lh_show_status_image == "Y" && ($live_help_opts.lh_show_operators_num == "Y" || $live_help_opts.lh_allow_start_chat == "Y" || $live_help_opts.lh_allow_leave_message == "Y")}
		<hr/>
	{/if}

	<div align="center">
		{if $live_help_opts.lh_allow_start_chat == "Y"}
			<div><a id="lh_start_chat_link" class="hidden">{$lang.lh_sidebox_start_chat}</a></div>
		{/if}
		{if $live_help_opts.lh_allow_leave_message == "Y"}
			<div><a href="javascript: live_help.leave_message();" id="lh_leave_message_link">{$lang.lh_sidebox_leave_message}</a></div>
		{/if}
	</div>
{/if}
<!--/dynamic-->
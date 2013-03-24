live_help_visitor = $.extend(live_help, {

	// Int: counter used to get operators activity (every 3 requests)
	counter: 0,

	// Flag: indicates first start
	first: true,

	// Flag: indicates that chat can be started
	start_chat: false,

	// Flag: indicates that invitation started
	start_invite: false,

	// Flag: indicates that visitor is invited to chat
	invited: false,

	// Int: indicates current operators status (online/offline, ready/busy)
	operator_status: -1,

	// Int: operator ID
	operator_id: 0,

	// Current action
	action: '',

	// Invitation messages to visitor
	inv_mess_id: 0,

	//
	// Initialize Live Help
	//
	init: function(params)
	{
		if (params) {
			for (k in params) {
				this[k] = params[k];
			}
		}

		this.send_request();
	},

	//
	// Enable/Disable links
	//
	enable_links: function(enable)
	{
		var links = ['lh_start_chat_link', 'lh_status_image_link'];
		var hrefs_online = ["javascript: live_help.request_chat()", "javascript: live_help.request_chat()"];
		var hrefs_offline = ["javascript: live_help.leave_message()", "javascript: live_help.leave_message()"];
//		var hrefs_offline = ["javascript: void(0)", "javascript: void(0)"];
		
		for (var n in links) {
			var a = document.getElementById(links[n]);
			if (!a) {
				continue;
			}
			
			if (enable == true) {
				if (links[n] == 'lh_start_chat_link') {
					a.style.display = '';
				}
				var href = hrefs_online[n];
			} else {
				if (links[n] == 'lh_start_chat_link') {
					a.style.display = 'none';
				}
				
				if (this.allow_leave_message == 'Y') {
					var href = hrefs_offline[n];
				} else {
					var href = '';
				}
			}
			
			if (href == '') {
				var content = $('img', $(a));
				content.attr('title', '');
				$(a).before(content);
				$(a).remove();
			} else {
				if (a.setAttribute) {
					a.setAttribute('href', href);
				} else {
					a.href = href;
				}
			}
		}
	},

	//
	// Send request to server
	//
	send_request: function(action)
	{
		if (typeof(action) == 'undefined') {
			action = (this.action == '') ?  '' : this.action;
		}
		if (this.action != '') {
			action = this.action;
			this.action = '';
		}

		// Indicates, call callback function or not to check the response
		var callback = true;
		var dispatch = '';
		var data = {};

		if (this.first == true) {
			dispatch = 'visitor.init';
			data['visitor_environment[title]']	 = "anonymous";
			data['visitor_environment[href]']	 = "mf href";//location.href;
			data['visitor_environment[browser]'] = "mf browser";//browser_info.browser + ", " + browser_info.version;
			data['visitor_environment[os]']		 = "mf os";//browser_info.os;
			data['visitor_environment[referer]'] = "mf referer";//document.referrer;
			data['first'] = 'Y';
		} else {
			dispatch = 'visitor.update';
		}

		dispatch = 'visitor.update';

		data['check_invitation'] = 'Y';
		if (this.invited == true) {
			data['check_invitation'] = 'N';
			callback = true;
		//	this.invited = false;
		}
	
		if (action == 'accept') {
			var today = new Date();
			data['local_time'] = Math.round(today.getTime() / 1000);
			data['user_name'] = this.user_name;
			data['operator_id'] = this.operator_id;
			data['check_invitation'] = 'Y';
		//	data['actions[invitation]'] = 'Y';
			callback = true;
			this.invited = false;
			dispatch = 'visitor.invite';
		} else if (action == 'decline') {
			data['invitation'] = 'N';
			this.invited = false;
			dispatch = 'visitor.invite';
		}
	
		var params = {
			data: data,
			hidden: true,
			cache: false,
			callback: (callback == true) ? [this, 'on_response']: {}
		};

		$.ajaxRequest(this.request_url + dispatch, params);

		this.first = (this.first == true) ? false : this.first;

		this.counter++;
		this.start_timer([this, 'send_request']);
	},

	//
	// Request response event
	//
	on_response: function(js, plain)
	{
		data = this.parse_xml(plain);
		if (!data) {
			return false;
		}

		var ops = data.getElementsByTagName('operators');

		if (ops.length) {
			// Total operators count recieved
			var op_count = parseInt(this.get_subnode_value(ops[0], 'total'));
			this.update_status('total', op_count);
			if (this.start_chat == true) {
				this.open_chat(op_count > 0);
				this.start_chat = false;
			}

			// Free operators count recieved
			var op_free = parseInt(this.get_subnode_value(ops[0], 'free'));
			this.update_status('free', op_free);
		}

		var inv_messages = data.getElementsByTagName('invitation_messages');

		if (inv_messages.length) {
			var im_id = this.get_subnode_value(inv_messages[0], 'first_inv_message_id');
			if (im_id && !this.inv_mess_id) {
				this.inv_mess_id = im_id;
			}
		}

		var ddd = inv_messages[0];
		var inv_arr = [];
		if (inv_messages.length) {
			for (var i = 0; i < ddd.childNodes.length; i++)	{
				if (this.get_subnode_value(ddd, 'inv_message_' + i) != '') {
					inv_arr[i] = this.get_subnode_value(ddd, 'inv_message_' + i);
					$('#invitation_text').text(this.get_subnode_value(ddd, 'inv_message_' + i));
					break;
				}
			}
		}

		var chat_data = data.getElementsByTagName('chat_data');

		if (chat_data.length) {
			this.invited = true;
			this.display_invitation_dialog({operator_id: this.get_subnode_value(chat_data[0], 'operator_id'), operator_name: this.get_subnode_value(chat_data[0], 'operator_name'), invitation_messages: inv_arr, visitor_name: this.get_subnode_value(chat_data[0], 'visitor_name')/*this.get_subnode_value(chat_data[0], 'invitation_message')*/});
		}

		return true;
	},

	send_request_invitation: function (action)
	{
		var callback = false;
		var dispatch = 'visitor.invite';
		var data = {};

		if (action == 'accept') {
			this.action = 'accept';
			var today = new Date();
			data['local_time'] = Math.round(today.getTime() / 1000);
			data['user_name'] = 'user_name';//this.user_name;
			data['operator_id'] = this.operator_id;
			data['invitation'] = 'Y';
		//	callback = true;
			this.invited = false;
		} else if (action == 'decline') {
			data['invitation'] = 'N';
			this.invited = false;
		}

		var params = {
			data: data,
			hidden: true,
			cache: false,
			callback: {}
		};

		$.ajaxRequest(this.request_url + dispatch, params);
	},

	send_request_chat: function (action)
	{
		this.open_chat(true);
	},

	//
	// Update LiveHelp status image
	//
	update_status: function (param, value)
	{
		if (param == 'total') {
			var total_count_element = document.getElementById('lh_total_count');
			if (total_count_element) {
				total_count_element.innerHTML = value;
			}
			
			var status_image_src = 'live_help_offline.gif';
			var status_image_hint = lang.image_offline_hint;
			var new_status = false;
			if (value > 0) {
				status_image_src = 'live_help_online.gif';
				status_image_hint = lang.image_online_hint;
				new_status = true;
			}
			var status_image = document.getElementById('lh_status_image');
			if (status_image && new_status != this.operator_status) {
				if (status_image.setAttribute) {
					status_image.setAttribute('src', this.status_image_path + status_image_src);
					status_image.setAttribute('title', status_image_hint);
				} else {
					status_image.src = this.status_image_path + status_image_src;
					status_image.title = status_image_hint;
				}
				status_image.style.display = 'inline';
				this.operator_status = new_status;
			}

			this.enable_links(new_status);

		} else if (param == 'free') {
			var free_count_element = document.getElementById('lh_free_count');
			if (free_count_element)	{
				free_count_element.innerHTML = value;
			}
		}
	},

	//
	// Request error handler
	//
	on_error: function()
	{
		this.update_status('free', 0);
		var total_count_element = document.getElementById('lh_total_count');
		if (total_count_element) {
			total_count_element.innerHTML = document.getElementById('lh_free_count').innerHTML = lang.not_available;
		}
		this.enable_links(false);
	},

	//
	// Send chat request to operator
	//
	request_chat: function()
	{
		if (!this.chatbox || this.chatbox && this.chatbox.closed) {
			this.start_chat = true;
			this.send_request_chat();
		} else if (this.chatbox) {
			this.chatbox.focus();
		}
	},

	//
	// Open chat window
	//
	open_chat: function(allow, force_enter)
	{
		if (typeof(this.opening_window) != 'undefined' && this.opening_window) {
			return;
		}
		
		this.opening_window = true;
		
		if (typeof(allow) == 'undefined') {
			allow = false;
		}
		if (typeof(force_enter) == 'undefined') {
			force_enter = false;
		}

		if (!allow)	{
			return this.leave_message();
		} else if (!this.chatbox || this.chatbox.closed) {
			this.inv_mess_id;

			var params = this.get_popup_params();
			var url = this.request_url + 'visitor.open_chat' + ((force_enter) ? '.force' : '') + '&mes_id=' + this.inv_mess_id;

			if (typeof(this.user_name) != 'undefined') {
				url +=  '&user_name=' + this.user_name;
			}

			this.chatbox = window.open(url, 'chat_window', params);

			this.inv_mess_id = 0;
		}
		
		if (!this.chatbox) {
			fn_alert(lang.failed_popup);
		}
		
		this.opening_window = false;
	},

	//
	// Open leave message window
	//
	leave_message: function ()
	{
		var params = {
			scrollbars: 'no',
			height: 370
		};
		
		this.message_box = window.open(this.request_url + 'visitor.leave_message', 1001, this.get_popup_params(params));
		if (!this.message_box) {
			fn_alert(lang.failed_popup);
		}
	},
	
	//
	// Align invitation dialog
	//
	display_invitation_dialog: function (data) 
	{
		if (!data) {
			return false;
		}

		var dlg = document.getElementById('invitation_dialog_bg');
		if (!dlg) {
			return false;
		}

		// set visitor name from session
		var inv_name_object = document.getElementById('invitation_name');
		if (inv_name_object && data.visitor_name)
		{
			inv_name_object.value = data.visitor_name;
		}

		this.operator_id = data.operator_id;

		$('#invitation_dialog_bg').show();
		$('#invitation_dialog_bg div').show();
		$('#invitation_dialog_bg').alignElement();
		var op_id = document.getElementById('op_id');
		if (!op_id)	{
			return false;
		}
		op_id.value = data.operator_id;

		var it = '';

		if (data.invitation_messages) {
		//	this.inv_mess = data.invitation_messages;
		//	this.inv_mess.push(data.invitation_message);
			it = data.operator_name + ': ';
			if (it) {
				for (k in data.invitation_messages) {
					if (data.invitation_messages[k] && data.invitation_messages[k] != '') {
						it += data.invitation_messages[k] + '\n';
					}
				}
			} else {
				for (k in data.invitation_messages) {
					if (data.invitation_messages[k] && data.invitation_messages[k] != '') {
						it += '\n' + data.invitation_messages[k] + '\n';
					}
				}
			}
		}
		$('#invitation_text').text(it);

		var intro = document.getElementById('invitation_intro');
		if (!intro)	{
			return false;
		}
		intro.innerHTML = lang.invitation_intro.str_replace('[operator]', data.operator_name);
	},

	//
	// Accept invitation to chat
	// 
	accept_invitation: function() 
	{
		var invitation_name_input = document.getElementById('invitation_name');
		this.user_name = invitation_name_input.value;
		if (!this.user_name) {
			fn_alert(lang.please_input_name);
			invitation_name_input.focus();
			return false;
		}

		this.action = 'accept';
		
		this.send_request_invitation('accept');
		$('#invitation_dialog_bg').hide();
		
		this.open_chat(true, true);
		
		return true;
	},

	//
	// Decline invitation to chat
	//
	decline_invitation: function() 
	{
		this.send_request_invitation('decline');
		$('#invitation_dialog_bg').hide();

		this.action = 'decline';

		return true;
	}
});
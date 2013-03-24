// Chat object
live_help_chat = $.extend(live_help, {
	
	MESSAGE_VISITOR: 0,
	MESSAGE_OPERATOR: 1,
	MESSAGE_SYSTEM: 2,

	// Flag: allow/disallow send message
	allow_send: false,

	// Flag: allow operator notification when typing the message
	allow_type_notify: false,

	// Int: ID of session
	session_id: null,

	// Int: ID of operator
	operator_id: null,

	// Int: ID of the latest message
	last_message: 0,

	// for scan invitation messages
	first_message: 0,

	// Array invitation messages
	inv_arr: Array(),

	//
	// Constructor
	//
	init: function(params, invitation_m)
	{
		if (params) {
			for (k in params) {

				this[k] = params[k];
			}
		}
		if (invitation_m) {
			for (k in invitation_m) {
				this.inv_arr[k] = invitation_m[k];
			}
		}

		var auth_name_object = document.getElementById('auth_name');
		if (auth_name_object && this.visitor_chat_name.length > 0) {
			auth_name_object.value = this.visitor_chat_name;
		}

		return (this.session_data) ? this.enter_chat(this.session_data) : true;
	},

	//
	// Enter the chat
	//
	enter_chat: function (force)
	{
		if (document.getElementById('enter_chat_submit').disabled == true) {
			return false;
		}

		if (typeof(force) == 'undefined') {
			force = false;
		}
		
		if (!force) {
			var auth_name_input = document.getElementById('auth_name');
			this.user_name = auth_name_input.value;
			if (!this.user_name) {
				auth_name_input.focus();
				return false;
			}

			$.disable_elms(['auth_name', 'enter_chat_submit'], true);

			var today = new Date();
			params = {
				data: {
					user_name: this.user_name,
					local_time: Math.round(today.getTime() / 1000)
				},
				cache: false,
				callback: [this, 'on_scan_messages']
			};


			$.ajaxRequest(this.request_url + 'dispatch=chat.request_chat', params);

			$.disable_elms(['message_text', 'send_button'], true);

			this.operator_name = '';
			this.operator_id = 0;
			this.show_message(lang.operator_wait, this.MESSAGE_SYSTEM);
		} else {
			$.disable_elms(['message_text', 'send_button'], true);
			// On starting chat by invitation
			this.session_id = force.visitor_id;

			this.user_name = force.user_name;
			this.operator_name = force.operator_name;
			this.last_message = force.last_message;
			for (k in this.inv_arr) {
//				this.show_message(this.inv_arr[k], this.MESSAGE_OPERATOR);

				var message = this.inv_arr[k]; // this.parse_xml(text);// (this.inv_arr[k][0] == '<') ? this.parse_xml(this.inv_arr[k]) : this.inv_arr[k];

				var text = this.inv_arr[k];
				text = text.replace(/&lt;/g, '<');
				text = text.replace(/&gt;/g, '>');

				if (text[0] == '<') {
					message = this.parse_xml('<text>' + text + '</text>');
					message = this.get_subnode_value(message, 'text');
				}

				this.show_message(message, this.MESSAGE_OPERATOR);
			}
		}

		$('#enter').hide();
		$('#chat').show();

		this.start_timer([this, 'scan_messages']);
		this.allow_type_notify = true;
		this.allow_send = true;
	},

	//
	// Покажем invitation messages
	//
	show_invitation_messages: function(inv_mess)
	{
		if (inv_mess == '') {
			return;
		}
		for (i = 0; i < inv_mess.length(); i++) {
			show_message(inv_mess[i], this.MESSAGE_OPERATOR);
		}
	},

	//
	// Event: on-typing notifier
	//
	on_typing: function(evt, text)
	{
		this.text = text;

		var char_code = (evt.which) ? evt.which : evt.keyCode;
		if (char_code == 13) {
			this.send_message();
		}

		if (this.allow_type_notify == false) {
			return false;
		}

		live_help_chat.switch_type_notify(1);

		if (this.typing_timer) {
			clearInterval(this.typing_timer);
		}

		typing_timer = setTimeout("live_help_chat.switch_type_notify(-1)", this.request_interval);
	},

	//
	// On-typing notifier helper
	//
	switch_type_notify: function(flag)
	{
		params = {
			data: {
				'session_id': this.session_id,
				'operator_id': this.operator_id
			},
			cache: false
		};
		
		$.ajaxRequest(this.request_url + 'dispatch=chat.' + flag, params);

		this.allow_type_notify = (flag != 1);	// flag can be 1 or -1
	},

	//
	// Scan for new messages
	//
	scan_messages: function()
	{
		var params = {
			data: {
			//	session_id: this.session_id,
				last_message: this.last_message,
				operator_id: this.operator_id
			},
			cache: false,
			hidden: true,
			callback: [this, 'on_scan_messages']
		};

		$.ajaxRequest(this.request_url + 'dispatch=chat.scan_messages', params);

		this.start_timer([this, 'scan_messages']);
	},

	//
	// Callback: on-scan messages handler
	//
	on_scan_messages: function(js, plain)
	{
		this.start_timer([this, 'scan_messages']);

		var data = this.parse_xml(plain);
		if (!data) {
			return false;
		}

		var session_data = data.getElementsByTagName('session_data');
		if (session_data.length) {
			this.session_id = parseInt(this.get_subnode_value(session_data[0], 'session_id'));
			return true;
		}

		// Checking operator's status
		var operator_info = data.getElementsByTagName('op');
		if (operator_info.length) {
			var val = this.get_subnode_value(operator_info[0], 'id');
			new_operator = (val != '-1') ? parseInt(val) : -1;
			
			if (new_operator != this.operator_id) {
				this.operator_id = new_operator;

				if (this.operator_id != -1) {
					$.disable_elms(['message_text', 'send_button'], false);
					this.operator_name = this.get_subnode_value(operator_info[0], 'name');
					this.show_message(lang.chat_operator_joined.replace(/\[operator\]/g, this.operator_name), this.MESSAGE_SYSTEM);
				} else {
					this.operator_id = 0;
					$.disable_elms(['message_text', 'send_button'], true);
					this.show_message(lang.chat_operator_left, this.MESSAGE_SYSTEM);
//					$('#type_notify').show();
				}
			}
		}

		// Checking typing notification
		if (data.getElementsByTagName('type_on').length) {
			$('#type_notify').show();
		} else if (data.getElementsByTagName('type_off').length) {
			$('#type_notify').hide();
		}

		// Recieving messages
		var messages = data.getElementsByTagName('message');
		for (var i = 0, len = messages.length; i < len; i++) {
			var message_id = this.get_subnode_value(messages[i], 'id');
			/*if (message_id <= this.last_message) { // FIXME? Don't work if chat is requested by visitor
				continue;
			}*/
			this.last_message = message_id;

			var direction = parseInt(this.get_subnode_value(messages[i], 'dir'));
			var message_text = this.get_subnode_value(messages[i], 'text');

			// Operator had sent the file
/*
			if (typeof(message_text) != 'string') {
				if (message_text[0] == 'file') {
					var file_name = message_text[1];
					var short_file_name = decodeURI(file_name.substr(file_name.search('_') + 1));
					var download_url = this.request_url + 'dispatch=chat.get_file&file_name=' + file_name;
					if ($.browser.msie) {
						download_url += "&encode_name=1"
					}
					var message_text = lang.chat_recieved_file.replace('[name]', short_file_name).replace('[url]', download_url);
					this.show_message(message_text, this.MESSAGE_SYSTEM);
				}
				if (message_text[0] == 'url') {
					var html_url = "<a href='" + message_text[1] + "' target='_blank'>" + message_text[1] + "</a>";
					if (direction == this.MESSAGE_OPERATOR) {
						html_url += " (" + lang.chat_opened_link + ")";
						setTimeout("live_help_chat.open_url('" + message_text[1] + "')", 1000);
					}
					this.show_message(html_url, direction);
				}
				if (message_text[0] == 'pic') {
					var html_pic = lang.chat_opened_link;
				//	setTimeout("live_help_chat.open_url('" + message_text[1] + "')", 1000);
					var file_name = this.request_url + 'dispatch=chat.get_pic&file_name=' + message_text[1];

					this.picBox = window.open(this.request_url + 'dispatch=chat.show_pic&file_name=' + message_text[1], 'pic from operator', 'width=400,height=300,toolbar=0');
					this.show_message(html_pic, direction);
				}

			} else {
*/
				if (direction == 1) {
					// message from operator
					this.show_message(message_text, this.MESSAGE_OPERATOR);
					$('#type_notify').hide();
				} else {
					// self message
					this.show_message(message_text, this.MESSAGE_VISITOR);
				}
//			}
		}
		
		// Actions after recieving message(s)
		if (messages.length) {
			window.focus();
			if (document.getElementById('message_text').disabled == false) {
				document.getElementById('message_text').focus();
			}
		}

		this.allow_send = true;
	},

	//
	// Display message
	//
	show_message: function(text, message_type)
	{

			message_text = text;
			if (typeof(message_text) != 'string') {
				var direction = message_type;

				if (message_text[0] == 'file') {
					var file_name = message_text[1];
					var short_file_name = decodeURI(file_name.substr(file_name.search('_') + 1));
					var download_url = this.request_url + 'dispatch=chat.get_file&file_name=' + file_name;
					if ($.browser.msie) {
						download_url += "&encode_name=1";
					}
					var message_text = lang.chat_recieved_file.replace('[name]', short_file_name).replace('[url]', download_url);
					this.show_message(message_text, this.MESSAGE_SYSTEM);
				}
				if (message_text[0] == 'url') {
					var html_url = "<a href='" + message_text[1] + "' target='_blank'>" + message_text[1] + "</a>";
					if (direction == this.MESSAGE_OPERATOR) {
						html_url += " (" + lang.chat_opened_link + ")";
						setTimeout("live_help_chat.open_url('" + message_text[1] + "')", 1000);
					}
					this.show_message(html_url, direction);
				}
				if (message_text[0] == 'pic') {
					var html_pic = lang.chat_opened_link;
				//	setTimeout("live_help_chat.open_url('" + message_text[1] + "')", 1000);
					var file_name = this.request_url + 'dispatch=chat.get_pic&file_name=' + message_text[1];

					this.picBox = window.open(this.request_url + 'dispatch=chat.show_pic&file_name=' + message_text[1], 'pic from operator', 'width=400,height=300,toolbar=0');
					this.show_message(html_pic, direction);
				}

				return;
			}


		text = text.replace(/\+\+\+lt;/g, '&lt;');
		text = text.replace(/\+\+\+;/g, '&amp;');
		if (message_type == this.MESSAGE_SYSTEM) {
			text = "<span class=\"system_message\">" + text + "</span><br />";
		} else if (message_type == this.MESSAGE_VISITOR) {
			text = "<span class=\"self_message\"><span class=\"name\">" + this.user_name + "</span>: " + text + "</span><br />";
		} else if (message_type == this.MESSAGE_OPERATOR) {
			text = "<span class=\"operator_message\"><span class=\"name\">" + this.operator_name + "</span>: " + text + "</span><br />";
		}
		
		document.getElementById('messages').innerHTML += text;
		document.getElementById('messages').scrollTop = document.getElementById('messages').scrollHeight;

		return true;
	},

	//
	// Send message
	//
	send_message: function()
	{
		if (this.text == '') {
			return false;
		}

		var text = this.text;
		if (document.getElementById('send_button').disabled == true) {
			return false;
		}

		if (this.allow_send == false) {
			return false;
		}

		document.getElementById('message_text').value = '';
		this.text = '';
		text = text.replace(/</gi, '&lt;');

		var params = {
			data: {
				'message_data[text]': text,
				'message_data[direction]': 0,
				'message_data[from_id]': this.session_id,
				'message_data[to_id]': this.operator_id
			},
			hidden: true,
			cache: false,
			callback: [this, 'scan_messages']
		};

		this.show_message(text, this.MESSAGE_VISITOR);

		$.ajaxRequest(this.request_url + 'dispatch=chat.send_message', params);

		return true;
	},

	//
	// Open url in new window
	//
	open_url: function(link)
	{
		link = link.replace(/\\\\/g, "\\\\");
		var win = open(link);
		win.focus();
	}
});
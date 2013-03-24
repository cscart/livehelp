live_help = {

	// Int: inverval between the requests (milliseconds)
	request_interval: 3000,

	// Invitation messages
	inv_mess: new Array(),
	
	start_timer: function(func)
	{
		if (this._timer) {
			clearInterval(this._timer);
		}

		this._timer = setTimeout(function() {
			func[0][func[1]]();
		}, this.request_interval);
	},

	construct_popup_params: function(params)
	{
		var param_arr = [];
		for (param in params) {
			param_arr[param_arr.length] = param + '=' + params[param];
		}
		
		return param_arr.join(',');
	},

	get_popup_params: function(overload)
	{
		var params = {
			width: 610,
			height: 445,
			status: 'yes',
			resizable: 'yes',
			scrollbars: 'no',
			location: 'no',
			inv_mess: Array()
		};
		params.screenX = (screen.width - params.width) / 2;
		params.screenY = (screen.height - params.height) / 2;
		// For Opera
		params.top = params.screenX - 30;
		params.left = params.screenY;
		
		if (typeof(overload) != 'undefined') {
			for (var param in overload) {
				params[param] = overload[param];
			}
		}
		
		return this.construct_popup_params(params);
	},
	
	parse_xml: function(xml)
	{
		if (xml.length == 0) {
			return false;
		}

		var data = {};
		if (window.ActiveXObject) {
			data = new ActiveXObject("Microsoft.XMLDOM");
			data.async = 'false';
			data.loadXML(xml);
		} else {
			var parser = new DOMParser();
			data = parser.parseFromString(xml, "text/xml");
		}

		return data;
	},

	get_subnode_value: function(parent_node, subnode_name)
	{
		var value = '';
		try {
			var node = parent_node.getElementsByTagName(subnode_name)[0].firstChild;
			if (node.nodeType == Node.TEXT_NODE) {
				value = node.nodeValue;
			} else {
				value = [node.tagName, node.firstChild.nodeValue];
			}
		} finally {
			return value;
		}
	}
};

if (!window.Node || !Node.ELEMENT_NODE) {
    Node = {
		ELEMENT_NODE: 1, 
		ATTRIBUTE_NODE: 2, 
		TEXT_NODE: 3, 
		CDATA_SECTION_NODE: 4, 
		ENTITY_REFERENCE_NODE: 5,  
		ENTITY_NODE: 6, 
		PROCESSING_INSTRUCTION_NODE: 7, 
		COMMENT_NODE: 8, 
		DOCUMENT_NODE: 9, 
		DOCUMENT_TYPE_NODE: 10, 
		DOCUMENT_FRAGMENT_NODE: 11, 
		NOTATION_NODE: 12
	};
}
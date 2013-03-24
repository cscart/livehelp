<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */

if ( !defined('AREA') )	{ die('Access denied');	}

if ($mode == "test") {
	header('Content-type: text/xml');
	die("OK-LIVEHELP-3.0");	
}

if ($mode == "file_upload") {
	set_time_limit(0);

	$visitor_id = ($_REQUEST['visitor_id'] != '') ? $_REQUEST['visitor_id'] : -1;
	$username = $_REQUEST['login'];
	$password = $_REQUEST['password'];
	$original_filename = $_REQUEST['filename'];

	if ($visitor_id < 0 || empty($username) || empty($password) || empty($original_filename)) {
		die('INVALID-PARAMS');
	}

	$operator = db_get_row("SELECT operator_id, name FROM ?:lh_operators WHERE login = ?s AND password = ?s AND status = 'A'", $username, $password);
	if (empty($operator)) {
		die('AUTH-FAILED');
	}

	$state = 2;
	$respond = 'DONE';

	$path = DIR_LH_UPLOADS;
	if (!is_dir($path))
		mkdir($path, 0777, true);

	$filename = md5(uniqid(rand(), true)) . '_' . str_replace(array('\\', '/', ':', '..'), '-', $original_filename);
	$fullpath =  $path . '/' . $filename;

	$data = array (
		'visitor_id'	=> $visitor_id,
		'date' => TIME,
		'original_filename'	=> $original_filename,
		'filename'	=> $filename,
		'path'	=> $path,
		'operator_id' => $operator['operator_id'],
		'operator_name' => $operator['name'],
		'state'	=> 1
	);
	$file_id = db_query('INSERT INTO ?:lh_files ?e', $data);


	if ($f = fopen($fullpath, "w+")) {
		$body = file_get_contents('php://input');
		fwrite($f, $body);
		fclose($f);
	}
	else
	{
		$respond = 'FILE-OPEN-ERROR';
		$state = 3; // error
	}

	$data = array (
		'state'	=> $state,
		'size'	=> filesize($fullpath),
		'date'	=> TIME
	);
	db_query("UPDATE ?:lh_files SET ?u WHERE file_id = ?i", $data, $file_id);

	die($respond);
}

if ($mode != "post") {
	die;
}

fn_trusted_vars('data');

if (!isset($_REQUEST["data"])) {
	die;
}

$xml = simplexml_load_string($_REQUEST["data"]);
if (empty($xml)) {
	die;
}

$operator = db_get_row("SELECT operator_id, name, obsolete_time, type_notify, last_message FROM ?:lh_operators WHERE login = ?s AND password = ?s AND status = ?s", (string)$xml->Auth->Username, (string)$xml->Auth->Password, 'A');

if (empty($xml->Auth->Username) || empty($xml->Auth->Password) || empty($operator)) {
	fn_lh_post_respond(LH_AUTH_RESULT_FAIL, ''); // Post AuthFailed respond and die
}


if (!empty($xml->Auth->Connection) && $operator['obsolete_time'] >= TIME) {
	fn_lh_post_respond(LH_AUTH_RESULT_ACTIVE, '');
}


// remove obsolete visotrs
cleanup_obsolete_visitors();

//
// Each authorized request from LiveHelp keep operator alive
//
db_query("DELETE FROM ?:lh_track WHERE user_id = ?i AND user_type = ?s", $operator["operator_id"], 'O');
		$t_data = array (
			'chat_track' => '',
			'track' => '',
			'avail' => 'Y',
			'user_type' => 'O',
			'user_id' => $operator["operator_id"]
		);
db_query("REPLACE INTO ?:lh_track ?e", $t_data);


// Update operator activity status
db_query('UPDATE ?:lh_operators SET ?u WHERE operator_id = ?i', array('obsolete_time' => (TIME + Registry::get('livehelp_config.freshness_time_operator'))), $operator['operator_id']);


// drop to Idle if no activity from the visitor
$visitors = db_get_array("SELECT visitor_id, obsolete_time_chat FROM ?:lh_visitors WHERE obsolete_time_chat < ?i AND status IN('".LH_STATUS_CHATTING."', '".LH_STATUS_WAITING."')", time());
if (!empty($visitors)) {
	foreach ($visitors as $visitor) {
		$data = array (
			'status' => LH_STATUS_IDLE,
			'operator_id' => 0
		);
		db_query("UPDATE ?:lh_visitors SET ?u WHERE visitor_id = ?i", $data, $visitor['visitor_id']);
	}
}

// process all chat messages received from the operator
fh_lh_process_chat_messages($xml, $operator);


// the XML respond body
$body_xml = '';

// send assigned operator name
$body_xml .= "  <Operator>\n";
$body_xml .= "    <Id>" . fn_lh_encode_xml_value($operator['operator_id']) . "</Id>\n";
$body_xml .= "    <Name>" . fn_lh_encode_xml_value($operator['name']) . "</Name>\n";
$body_xml .= "  </Operator>\n";


// process notes 
fn_lh_visitor_notes_xml($xml->Notes, $operator);

// process uploaded files
fn_lh_uploaded_files_xml($xml->Files, $operator);


// add visitors track info to the XML respond
$body_xml .= fh_lh_get_visitors_info_xml($xml->Visitors);

// add visitors chat messages to the XML respond
$body_xml .= fn_lh_get_visitors_chat_msgs_xml($operator);


// chat log respond
$body_xml .= fn_lh_get_visitors_chat_log_msgs_xml($xml->Visitors, $operator);

// event log respond
$body_xml .= fn_lh_get_visitors_event_records_log_xml($xml->Visitors);

// stickers respond
$body_xml .= fn_lh_stickers_xml($xml->Stickers, $operator);

// offline messages respond
$body_xml .= fn_lh_offline_messages_xml($xml->OfflineMessages, $operator);


// post XML respond
fn_lh_post_respond(LH_AUTH_RESULT_OK, $body_xml);



die;



//////////////////////////////////////////////// Helper functions ////////////////////////////////////////////////////

function fn_lh_visitor_notes_xml($object, $operator)
{

	foreach($object->Note as $note) {
		$type = (string)$note['Type'];

		switch ($type)
		{
			case "Hash":
			break;

			case "New":
				$message = (string)$note->Message;
				$visitor_id = (int)$note->VisitorId;

				$data = array (
					'visitor_id'	=> $visitor_id,
					'operator_id' => $operator['operator_id'],
					'operator_name' => $operator['name'],
					'date' => TIME,
					'note' => $message,
				);
				db_query('INSERT INTO ?:lh_visitors_notes ?e', $data);
			continue;

			case "Obsolete":
				$id = (string)$note['Id'];
				db_query("DELETE FROM ?:lh_visitors_notes WHERE note_id = ?i", $id);
			continue;
		}
	}
}

function fn_lh_uploaded_files_xml($object, $operator)
{
	foreach($object->File as $file) {
		$type = (string)$file['Type'];
		$id = (string)$file['Id'];

		switch ($type)
		{
			case "Hash":
			break;

			case "Post":
				$row = db_get_row('SELECT filename, visitor_id FROM ?:lh_files WHERE file_id = ?i', $id);
				$message = '<file>' . $row['filename'] . '</file>';

				fh_lh_post_chat_message($operator['operator_id'], $row['visitor_id'], $message, LH_DIRECTION_OPERATOR_VISITOR);
			continue;

			case "Obsolete":
				$row = db_get_row('SELECT path, filename FROM ?:lh_files WHERE file_id = ?i', $id);
				if (!empty($row)) {
					$filename = $row['path'] . '/' . $row['filename'];
					@unlink($filename);
				}
				db_query("DELETE FROM ?:lh_files WHERE file_id = ?i", $id);
			continue;
		}
	}
}


function fh_lh_process_chat_messages($xml, $operator)
{
	if (!is_object($xml->ChatMessages) || $xml->ChatMessages == null || count($xml->ChatMessages) <= 0) {
		return false;
	}

	foreach($xml->ChatMessages->Message as $message) {
		$visitor_id = (int)$message["VisitorId"];
		$value = (string)$message;

		$operator_id = $operator["operator_id"];

/*
		$status = db_get_field('SELECT status FROM ?:lh_visitors WHERE visitor_id = ?i', $visitor_id);

		// Request for a chat if visitor's status is Idle and message received
		if ($status == LH_STATUS_IDLE) {
			fn_lh_request_chat($operator_id, $visitor_id);
		}

		// start chat if visitor wait it
		if ($status == LH_STATUS_WAITING) {
			fn_lh_start_chat($operator_id, $visitor_id);
		}

		fn_lh_chat_message($operator_id, $visitor_id, $value);
//*/

		fh_lh_post_chat_message($operator_id, $visitor_id, $value);
	}

	return true;
}

function fh_lh_post_chat_message($operator_id, $visitor_id, $message, $direction=LH_DIRECTION_OPERATOR_VISITOR)
{
	$status = db_get_field('SELECT status FROM ?:lh_visitors WHERE visitor_id = ?i', $visitor_id);

	// Request for a chat if visitor's status is Idle and message received
	if ($status == LH_STATUS_IDLE) {
		fn_lh_request_chat($operator_id, $visitor_id);
	}

	// start chat if visitor wait it
	if ($status == LH_STATUS_WAITING) {
		fn_lh_start_chat($operator_id, $visitor_id);
	}

	fn_lh_chat_message($operator_id, $visitor_id, $message, $direction);
}

function fn_lh_request_chat($operator_id, $visitor_id)
{
	$data = array (
		'operator_id' => $operator_id,
		'status' => LH_STATUS_REQUEST_SENT
	);
	db_query('UPDATE ?:lh_visitors SET ?u WHERE visitor_id = ?i', $data, $visitor_id);

	fn_add_log_event(array('id' => $visitor_id), LH_LOG_SENT_INVITATION, $operator_id);
}

function fn_lh_start_chat($operator_id, $visitor_id)
{
	$data = array (
		'operator_id' => $operator_id,
		'status' => LH_STATUS_CHATTING
	);
	db_query('UPDATE ?:lh_visitors SET ?u WHERE visitor_id = ?i', $data, $visitor_id);

	fn_add_log_event(array('id' => $visitor_id), LH_LOG_OPERATOR_JOINED, $operator_id);
	fn_add_log_event(array('id' => $visitor_id), LH_LOG_CHAT_STARTED, $operator_id);
}

function fn_lh_chat_message($operator_id, $visitor_id, $message, $direction=LH_DIRECTION_OPERATOR_VISITOR)
{
	$data = array (
		'direction' => $direction,
		'from_id' => $operator_id,
		'to_id' => $visitor_id,
		'text' => $message,
		'date' => time(),
	);
	db_query('INSERT INTO ?:lh_messages ?e', $data);
}

function fn_lh_get_visitors_chat_msgs_xml($operator)
{
	$operator_id = $operator["operator_id"];

	$last_message_id = db_get_field("SELECT max(message_id) AS message_id FROM ?:lh_messages WHERE ((from_id = ?i AND direction = ?i) OR (to_id = ?i AND direction = ?i))", $operator_id, LH_DIRECTION_OPERATOR_VISITOR, $operator_id, LH_DIRECTION_VISITOR_OPERATOR);
	if ($operator['last_message'] > $last_message_id)
	{
		$operator['last_message'] = $last_message_id;
		db_query("UPDATE ?:lh_operators SET last_message = ?i WHERE operator_id = ?i", $last_message_id, $operator_id);
		return "";
	}


	// Selecting new messages from all users
	$condition = db_quote("message_id > ?i AND (to_id = ?i OR from_id = ?i) AND direction = ?i ORDER BY date", $operator['last_message'], $operator_id, $operator_id, LH_DIRECTION_VISITOR_OPERATOR);
	$fields = array (
		'message_id',
		'date',
		'direction',
		'from_id',
		'to_id',
		'text'
	);
	$messages = db_get_array("SELECT " . implode(', ', $fields) . " FROM ?:lh_messages WHERE $condition");

	$xml = '';

	if (!empty($messages)) {
		$last_message = -1;

		$xml .= "  <ChatMessages>\n";
		foreach ($messages as $message) {
			$xml .= "    <Message Id='".fn_lh_encode_xml_value($message['message_id'])."' VisitorId='" . fn_lh_encode_xml_value($message['from_id']) . "'>\n";
			$xml .= "      <Date>" . fn_lh_encode_xml_value($message['date']) . "</Date>\n";
			$xml .= "      <Text>" . fn_lh_encode_xml_value($message['text']) . "</Text>\n";
			$xml .= "    </Message>\n";

			$last_message = $message['message_id'];
		}

		$xml .= "  </ChatMessages>\n";

		if ($last_message > 0) {
			db_query("UPDATE ?:lh_operators SET last_message = ?i WHERE operator_id = ?i", $last_message, $operator_id);
		}
	}

	return $xml;
}

function fn_lh_get_visitors_chat_log_msgs_xml($visitors_object, $operator)
{
	$result = array();

	if (is_object($visitors_object) && $visitors_object != null && count($visitors_object->Visitor) > 0) {
		foreach($visitors_object->Visitor as $visitor) {
			$operator_id = $operator["operator_id"];
			$visitor_id = (string)$visitor['Id'];

			if ($visitor->LastChatMessageId == '')
			{
				continue;
			}

			$message_id = $visitor->LastChatMessageId;

			// Selecting new messages from all users
			$condition = db_quote("message_id > ?i AND ((to_id = ?i AND from_id = ?i AND direction = ?i) OR (to_id = ?i AND from_id = ?i AND direction = ?i)) ORDER BY message_id LIMIT 20", $message_id, $operator_id, $visitor_id, LH_DIRECTION_VISITOR_OPERATOR, $visitor_id, $operator_id, LH_DIRECTION_OPERATOR_VISITOR);
			$fields = array (
				'message_id',
				'date',
				'direction',
				'from_id',
				'to_id',
				'text'
			);
			$messages = db_get_array("SELECT " . implode(', ', $fields) . " FROM ?:lh_messages WHERE $condition");	

			if (is_array($messages) && count($messages) > 0)
			{
				$result[$visitor_id] = $messages;
			}
		}
	}


	$xml = "";

	if (count($result) > 0) {
		foreach ($result as $visitor_id=>$messages)
		{
			$xml .= "  <ChatMessagesLog>\n";
			foreach ($messages as $message) {
				$xml .= "    <Message Id='".fn_lh_encode_xml_value($message['message_id'])."'>\n";
				$xml .= "      <FromId>" . fn_lh_encode_xml_value($message['from_id']) . "</FromId>\n";
				$xml .= "      <ToId>" . fn_lh_encode_xml_value($message['to_id']) . "</ToId>\n";
				$xml .= "      <Direction>" . fn_lh_encode_xml_value($message['direction']) . "</Direction>\n";
				$xml .= "      <Date>" . fn_lh_encode_xml_value($message['date']) . "</Date>\n";
				$xml .= "      <Text>" . fn_lh_encode_xml_value($message['text']) . "</Text>\n";
				$xml .= "    </Message>\n";
			}

			$xml .= "  </ChatMessagesLog>\n";
		}

	}

	return $xml;
}

function fn_lh_get_visitors_event_records_log_xml($visitors_object)
{
	$result = array();

	if (is_object($visitors_object) && $visitors_object != null && count($visitors_object->Visitor) > 0) {
		foreach($visitors_object->Visitor as $visitor) {
			$visitor_id = (string)$visitor['Id'];

			if ($visitor->LastEventRecordId == '' || empty($visitor_id))
			{
				continue;
			}

			$log_id = $visitor->LastEventRecordId;

			// Selecting new messages from all users
			$condition = db_quote("visitor_id = ?i AND log_id > ?i ORDER BY log_id LIMIT 20", $visitor_id, $log_id);
			$fields = array (
				'log_id',
				'visitor_id',
				'date',
				'event',
				'operator_id',
				'operator_name',
				'page_url',
				'page_title'
			);
			$events = db_get_array("SELECT " . implode(', ', $fields) . " FROM ?:lh_visitors_log WHERE $condition");	

			if (is_array($events) && count($events) > 0)
			{
				$result[$visitor_id] = $events;
			}
		}
	}


	$xml = "";

	if (count($result) > 0) {
		foreach ($result as $visitor_id=>$events)
		{
			$xml .= "  <EventsLog>\n";
			foreach ($events as $event) {
				$xml .= "    <Event Id='".fn_lh_encode_xml_value($event['log_id'])."'>\n";
				$xml .= "      <VisitorId>" . fn_lh_encode_xml_value($event['visitor_id']) . "</VisitorId>\n";
				$xml .= "      <Code>" . fn_lh_encode_xml_value($event['event']) . "</Code>\n";
				$xml .= "      <Date>" . fn_lh_encode_xml_value($event['date']) . "</Date>\n";
				$xml .= "      <OperatorId>" . fn_lh_encode_xml_value($event['operator_id']) . "</OperatorId>\n";
				$xml .= "      <OperatorName>" . fn_lh_encode_xml_value($event['operator_name']) . "</OperatorName>\n";
				$xml .= "      <PageURL>" . fn_lh_encode_xml_value($event['page_url']) . "</PageURL>\n";
				$xml .= "      <PageTitle>" . fn_lh_encode_xml_value($event['page_title']) . "</PageTitle>\n";
				$xml .= "    </Event>\n";
			}

			$xml .= "  </EventsLog>\n";
		}

	}

	return $xml;
}

function fn_lh_stickers_xml($object, $operator)
{
	$stickers_map = array();

	if (is_object($object) && $object != null && count($object->Sticker) > 0) {
		foreach($object->Sticker as $sticker) {
			$key = (string)$sticker['Id'];
			$type = (string)$sticker['Type'];
			$hash = (string)$sticker->Hash;

			switch ($type)
			{
				case "Hash":
				break;

				case "New":
					$title = (string)$sticker->Title;
					$message = (string)$sticker->Message;

					$data = array (
						'operator_id' => $operator['operator_id'],
						'operator_name' => $operator['name'],
						'date' => TIME,
						'title' => $title,
						'text' => $message,
					);
					db_query('INSERT INTO ?:lh_stickers ?e', $data);
				continue;

				case "Obsolete":
					db_query("DELETE FROM ?:lh_stickers WHERE sticker_id = ?i", $key);
				continue;
			}

			$stickers_map[$key] = array('hash' => $hash);
		}
	}


	$sticker_fields = array (
		'sticker_id',
		'operator_id',
		'operator_name',
		'date',
		'title',
		'text',
	);

	$stickers_temp = db_get_array("SELECT " . implode(', ', $sticker_fields) . " FROM ?:lh_stickers");

	$stickers = array();
	foreach ($stickers_temp as $sticker)
	{
		$id = $sticker["sticker_id"];
		$sticker['hash'] = md5(serialize($sticker));


		if (!empty($stickers_map[$id])) {

			$hash = $stickers_map[$id]['hash'];
			unset($stickers_map[$id]);

			if ($sticker["hash"] == $hash) {
				continue;
			}
		}

/*
		// update visitor's name by operator
		if (!empty($name)) {
			db_query("UPDATE ?:lh_visitors SET assigned_name = ?s WHERE visitor_id = ?i", $name, $id);
			unset($visitor['hash']);
			$visitor['visitor_name'] = $name;
			$visitor['hash'] = md5(serialize($visitor));
		}
//*/


		// add or update visitor required
		$stickers[] = $sticker;
	}



	$alias_fields = array(
		'sticker_id' => 'Id',
		'operator_id' => 'OperatorId',
		'operator_name' => 'OperatorName',
		'status' => 'Status',
		'date' => 'Date',
		'title' => 'Title',
		'text' => 'Message',
		'hash' => 'Hash'
	);



	$xml = '';
	if (!empty($stickers)) {
		$xml .= "  <Stickers>\n";
		foreach ($stickers as $sticker) {
			$xml .= "    <Sticker>\n" . fn_lh_array_to_xml($sticker, $alias_fields, '', 6) . "    </Sticker>\n";
		}
		$xml .= "  </Stickers>\n";
	}

	if (!empty($stickers_map)) {
		$xml .= "  <ObsoleteStickers>\n";
		foreach ($stickers_map as $id=>$sticker) {
			$xml .= "    <Sticker Id='" . fn_lh_encode_xml_value($id) . "' />\n";
		}
		$xml .= "  </ObsoleteStickers>\n";
	}

	return $xml;
}

function fn_lh_offline_messages_xml($object, $operator)
{

	$messages_map = array();

	if (is_object($object) && $object != null && count($object->Message) > 0) {
		foreach($object->Message as $message) {
			$key = (string)$message['Id'];
			$type = (string)$message['Type'];
			$hash = (string)$message->Hash;

			switch ($type)
			{
				case "Hash":
				break;

				case "Reply":
					$reply = (string)$message->Reply;
					$subject = (string)$message->Subject;
					$to_email = (string)$message->ToEmail;

					$view_mail = & Registry::get('view_mail');
					$view_mail->assign('body', nl2br($reply));
					$view_mail->assign('subject', $subject);
				
					fn_send_mail($to_email, Registry::get('settings.Company.company_users_department'), 'addons/live_help/mail_subj.tpl', 'addons/live_help/mail.tpl');

					$_data = array (
						'reply' => $reply,
						'replied' => 1,
						'reply_date' => TIME,
						'operator_id' => $operator['operator_id'],
						'operator_name' => $operator['name'],
					);

					db_query('UPDATE ?:lh_left_messages SET ?u WHERE message_id = ?i', $_data, $key);
				continue;

				case "Obsolete":
					db_query("DELETE FROM ?:lh_left_messages WHERE message_id = ?i", $key);
				continue;
			}

			$messages_map[$key] = array('hash' => $hash);
		}
	}




	$message_fields = array (
		'message_id',
		'date',
		'reply_date',
		'from_name',
		'from_mail',
		'subject',
		'message',
		'operator_id',
		'operator_name',
		'reply',
		'replied'
	);

	$messages_temp = db_get_array("SELECT " . implode(', ', $message_fields) . " FROM ?:lh_left_messages");	


	$messages = array();
	foreach ($messages_temp as $message)
	{
		$id = $message["message_id"];

		$message['status'] = 0;
		if ($message['reply'] != '')
			$message['status'] = (($message['replied'] > 0) ? 2 : 1); // 2 - sent; 1 - pending

		$message['hash'] = md5(serialize($message));


		if (!empty($messages_map[$id])) {
			$hash = $messages_map[$id]['hash'];
			unset($messages_map[$id]);

			if ($message["hash"] == $hash) {
				continue;
			}
		}


		// add or update message required
		$messages[] = $message;
	}


	$alias_fields = array(
		'message_id' => 'Id',
		'date' => 'Date',
		'reply_date' => 'ReplyDate',
		'from_name'	=> 'FromName',
		'from_mail'	=> 'FromEmail',
		'subject' => 'Subject',
		'message' => 'Message',
		'operator_id' => 'OperatorId',
		'operator_name' => 'OperatorName',
		'reply' => 'Reply',
		'status' => 'Status',
		'hash' => 'Hash'
	);

	$xml = "";
	if (!empty($messages)) {
		$xml .= "  <OfflineMessages>\n";
		foreach ($messages as $message) {
			$xml .= "    <Message>\n" . fn_lh_array_to_xml($message, $alias_fields, '', 6) . "    </Message>\n";
		}
		$xml .= "  </OfflineMessages>\n";
	}

	if (!empty($messages_map)) {
		$xml .= "  <ObsoleteOfflineMessages>\n";
		foreach ($messages_map as $id=>$message) {
			$xml .= "    <Message Id='" . fn_lh_encode_xml_value($id) . "' />\n";
		}
		$xml .= "  </ObsoleteOfflineMessages>\n";
	}

	return $xml;
}


function fh_lh_get_visitors_info_xml($object)
{
	$alias_fields = array(
		'visitor_id' => 'Id', 
		'ip' => 'Ip', 
//		'visitor_name' => 'Name',
		'visitor_name' => 'NameByOperator',
		'chat_name' => 'NameByVisitor',
		'status' => 'Status',
		'operator_id' => 'OperatorId', 
		'last_message' => 'LastMessage', 
		'type_notify' => 'TypeNotify', 
		'os' => 'Os',
		'referrer' => 'Referrer', 
		'operator_name' => 'OperatorName', 
		'url' => 'Url', 
		'title' => 'Title',
		'browser' => 'Browser', 
		'country' => 'Country', 
		'hash' => 'Hash',
		'notes' => 'Notes',
		'id'	=> 'Id',
		'date'	=> 'Date',
		'operator_id'	=> 'OperatorId',
		'operator_name'	=> 'OperatorName',
		'note'	=> 'Message',
		'filename'	=> 'Filename',
		'size'	=> 'Size',
		'state'	=> 'State'
	);

	$visitors_map = array();

	if (is_object($object) && $object != null && count($object->Visitor) > 0) {
		foreach($object->Visitor as $visitor) {
			$key = (string)$visitor['Id'];

			$hash = (string)$visitor->Hash;
			$name = (string)$visitor->Name;

			$visitors_map[$key] = array('hash' => $hash, 'name' => $name);
		}
	}


	$visitor_fields = array (
		'visitor_id',
		'session_id',
		'assigned_name as visitor_name',
		'chat_name',
		'status',
		'operator_id',
		'ip',
	);

	$visitors = array();
	$visitors_temp = db_get_array("SELECT " . implode(', ', $visitor_fields) . " FROM ?:lh_visitors WHERE obsolete_time >= ?i", TIME);
	foreach ($visitors_temp as $visitor) {
		$id = $visitor["visitor_id"];

		// visitor's name for operator only
		if ($visitor['visitor_name'] == '') {
			$visitor['visitor_name'] = ($visitor['chat_name'] == '') ? 'Guest' : $visitor['chat_name'];
		}

		$condition = db_quote('operator_id = ?i', $visitor["operator_id"]);
		$visitor["operator_name"] = db_get_field("SELECT name FROM ?:lh_operators WHERE operator_id = ?i", $visitor["operator_id"]);

//		$visitor_ip = db_get_field("SELECT ip FROM ?:lh_visitors WHERE visitor_id = ?i", $id);
		$stat_sess_id = db_get_field("SELECT MAX(sess_id) FROM ?:stat_sessions WHERE session = ?s", $visitor['session_id']);

		// get OS, refferer and browser_id then merge with visitor
//		$extra_data = db_get_row("SELECT os, referrer, browser_id FROM ?:stat_sessions WHERE host_ip = ?s AND sess_id = ?i", $visitor_ip, $stat_sess_id);
		$extra_data = db_get_row("SELECT os, referrer, browser_id FROM ?:stat_sessions WHERE sess_id = ?i", $stat_sess_id);
		$visitor = array_merge($visitor, $extra_data);


		// get URL and title then merge with visitor
		$extra_data = db_get_row("SELECT url, title, https FROM ?:stat_requests WHERE sess_id = ?i ORDER BY timestamp DESC LIMIT 1", $stat_sess_id);
		$visitor = array_merge($visitor, $extra_data);

		// complete current visitor URL
		$visitor['url'] = Registry::get(($visitor['https'] == 'Y') ? 'config.https_location' : 'config.http_location') . $visitor['url'];

		// get country and browser info then merge
		$browser = db_get_row("SELECT browser, version FROM ?:stat_browsers WHERE browser_id = ?i", $visitor['browser_id']);
		$visitor['browser'] = $browser['browser'] . ' ' . $browser['version'];
		$visitor['country'] = fn_get_country_by_ip($visitor["ip"]);

		// get notes
		$notes = db_get_array("SELECT note_id as id, operator_id, operator_name, date, note FROM ?:lh_visitors_notes WHERE visitor_id = ?i ORDER BY date", $id);
		$visitor['notes'] = $notes;

		// get files
		$files = db_get_array("SELECT file_id as id, date, original_filename as filename, size, operator_id, operator_name, state FROM ?:lh_files WHERE visitor_id = ?i ORDER BY date", $id);
		$visitor['files'] = $files;


		$visitor["hash"] = md5(serialize($visitor));

		$name = '';
		if (!empty($visitors_map[$id])) {

			$hash = $visitors_map[$id]['hash'];
			$name = $visitors_map[$id]['name'];
			unset($visitors_map[$id]);

			if ($visitor["hash"] == $hash && strlen($name) == 0) {
				continue;
			}
		}


		// update visitor's name by operator
		if (!empty($name)) {
			db_query("UPDATE ?:lh_visitors SET assigned_name = ?s WHERE visitor_id = ?i", $name, $id);
			unset($visitor['hash']);
			$visitor['visitor_name'] = $name;
			$visitor['hash'] = md5(serialize($visitor));
		}

		// add or update visitor required
		$visitors[] = $visitor;
	}

	// now,
	// $visitors - contain list of all add/update visitors
	// $visitors_map - contain list of all obsolete visitors
	// compose XML and return

	$xml = '';
	if (!empty($visitors)) {
		$xml .= "  <Visitors>\n";
		foreach ($visitors as $visitor) {
			$xml .= "    <Visitor Id='" . fn_lh_encode_xml_value($visitor['visitor_id']) . "'>\n";
			$xml .= fn_lh_array_to_xml($visitor, $alias_fields, '', 6);

			// Notes
			if (!empty($visitor['notes']))
			{
				$xml .= "      <Notes>\n";
					foreach ($visitor['notes'] as $note) {
						$xml .= "        <Note>\n"; // Id='" . fn_lh_encode_xml_value($note['note_id']) . "'>\n";
						$xml .= fn_lh_array_to_xml($note, $alias_fields, '', 10);
						$xml .= "        </Note>\n";
					}
				$xml .= "      </Notes>\n";
			}

			// Files
			if (!empty($visitor['files']))
			{
				$xml .= "      <Files>\n";
					foreach ($visitor['files'] as $file) {
						$xml .= "        <File>\n"; // Id='" . fn_lh_encode_xml_value($file['file_id']) . "'>\n";
						$xml .= fn_lh_array_to_xml($file, $alias_fields, '', 10);
						$xml .= "        </File>\n";
					}
				$xml .= "      </Files>\n";
			}

			$xml .= "    </Visitor>\n";
		}
		$xml .= "  </Visitors>\n";
	}

	if (!empty($visitors_map)) {
		$xml .= "  <ObsoleteVisitors>\n";
		foreach ($visitors_map as $id=>$visitor) {
			$xml .= "    <Visitor Id='" . fn_lh_encode_xml_value($id) . "' />\n";
		}
		$xml .= "  </ObsoleteVisitors>\n";
	}

	return $xml;
}



function fn_lh_array_to_xml($array, $alias_fields=array(), $xml_str="", $offset=2) {
	foreach ($array as $k => $v) {
		if (empty($alias_fields[$k])) {
			continue;
		}

		$key = (strlen($alias_fields[$k]) > 0) ? $alias_fields[$k] : $k;

		if (is_array($v)) {
/*
			$xml_str .= str_repeat(' ', $offset) . "<$key>\n";
			$offset += 2;
			foreach ($v as $value) {
				$xml_str .= fn_lh_array_to_xml($value, $alias_fields, "", $offset);
			}
			$offset -= 2;
			$xml_str .= str_repeat(' ', $offset) . "</$key>\n";
//*/
			continue;
		}

		$xml_str .= str_repeat(' ', $offset) . "<$key>" . fn_lh_encode_xml_value($v) . "</$key>\n";
	}

	return $xml_str;
}

function fn_lh_encode_xml_value($value) {
	if (strpos($value, '&') === false && strpos($value, '<') === false) {
		return $value;
	}

	$value = str_replace(']]>', '', $value);
	return '<![CDATA[' . $value . ']]>';
}


function fn_lh_post_respond($auth_result, $respond_body) {
	header('Content-type: text/xml');

	switch ($auth_result)
	{
		case LH_AUTH_RESULT_OK:
			$auth = 'OK';
		break;

		case LH_AUTH_RESULT_ACTIVE:
			$auth = 'ACTIVE';
			$respond_body = '';
		break;

		default:
		case LH_AUTH_RESULT_FAIL:
			$auth = 'FAIL';
			$respond_body = '';
		break;
	};

	$respond = <<<EOT
<?xml version='1.0' encoding='UTF-8'?>
<RootXmlRespond>
  <AuthResult>$auth</AuthResult>
$respond_body
</RootXmlRespond>
EOT;


	die($respond);
}


die;
?>
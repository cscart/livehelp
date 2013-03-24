<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */


if ( !defined('AREA') )	{ die('Access denied');	}

//
// Remove LiveHelp information
//
if ($mode == 'delete') {

	db_query("TRUNCATE TABLE ?:lh_left_messages");
	db_query("TRUNCATE TABLE ?:lh_messages");
	db_query("TRUNCATE TABLE ?:lh_messages_map");
	db_query("TRUNCATE TABLE ?:lh_sessions");
	db_query("TRUNCATE TABLE ?:lh_type_notify");
	db_query("TRUNCATE TABLE ?:lh_visitors");
	db_query("TRUNCATE TABLE ?:lh_visitors_log");
	db_query("TRUNCATE TABLE ?:lh_visitors_notes");

	return array(CONTROLLER_STATUS_OK, "chat_history.manage");

//
// 'Management' page
//
} elseif ($mode == 'manage') {

	$k = 0;
	$texts = array();

	$operators_assoc = db_get_hash_array("SELECT operator_id AS key_field, operator_id AS id, login, name FROM ?:lh_operators ORDER BY login", 'key_field');

	// Init filter
	$params = fn_init_view('chat_history', $_REQUEST);

	$params['operator'] = (!empty($params['operator'])) ? $params['operator'] : 'all';
	$params['match'] = (!empty($params['match'])) ? $params['match'] : 'any';
	$params['q'] = (!empty($params['q'])) ? $params['q'] : '';
	$params['name'] = (!empty($params['name'])) ? $params['name'] : '';
	$params['email'] = (!empty($params['email'])) ? $params['email'] : '';
	$params['group'] = (!empty($params['group'])) ? $params['group'] : 'all';
	$params['page'] = (!empty($params['page'])) ? $params['page'] : 1;

	$_where = '';
	if (!empty($params['period']) && $params['period'] != 'A') {
		list($params['time_from'], $params['time_to']) = fn_create_periods($params);
		$_where = "(date >= '$params[time_from]' AND date < '$params[time_to]') AND ";
	}

	$where_chat = $where_operators = $where_messages = $_where;


	// Generating history categories

	$where_search = '1';
	if ($params['q']) {
		$words = explode(' ', $params['q']);
		if ($params['match'] == 'any') {
			$where_search = array();
			foreach ($words as $word) {
				$where_search[] = "{search_field} LIKE '%$word%'";
			}
			$where_search = implode(' OR ', $where_search);
		} elseif ($params['match'] == 'all') {
			$where_search = array();
			foreach ($words as $word) {
				$where_search[] = "{search_field} LIKE '%$word%'";
			}
			$where_search = implode(' AND ', $where_search);
		} elseif ($params['match'] == 'exact') {
			$where_search = "{search_field} LIKE '%$params[q]%'";
		}
	}

	$persons = array();

	if ($params['group'] == 'all' || $params['group'] == 'chat') {
		// WHERE of query
		if ($params['operator'] != 'all') {
			$where_chat .= "(direction = '" . LH_DIRECTION_VISITOR_OPERATOR . "' AND to_id = '$params[operator]') OR (direction = '" . LH_DIRECTION_OPERATOR_VISITOR . "' AND from_id = '$params[operator]')";
		} else {
			$where_chat .= "(direction IN ('" . LH_DIRECTION_VISITOR_OPERATOR . "', '" . LH_DIRECTION_OPERATOR_VISITOR . "'))";
		}
		// search part in WHERE
		if ($where_search) {
			$search_query = str_replace('{search_field}', 'text', $where_search);
			$where_chat = "($where_chat) AND ($search_query)";
		}

		// Getting chat sessions
		$operator_name = array();
		$session_data = db_get_array("SELECT direction, from_id, to_id, date, text FROM ?:lh_messages WHERE $where_chat ORDER BY date");
		$session_ids = array();
		$session_info = array();
		foreach ($session_data as $row) {
			list($direction, $from_id, $to_id, $date) = array($row['direction'], $row['from_id'], $row['to_id'], $row['date']);
			$id = ($direction == LH_DIRECTION_VISITOR_OPERATOR) ? $from_id : $to_id;
			$op_id = ($direction == LH_DIRECTION_OPERATOR_VISITOR) ? $from_id : $to_id;
			if ($params['operator'] != 'all') {
				$op_id = $params['operator'];
			}

			if($params['q']) {
				$operator_name = db_get_row("SELECT login, name FROM ?:lh_operators WHERE operator_id = ?i", $from_id);
				$texts[$k]['operator_name'] = $operator_name['login'].' ('.$operator_name['name'].')';
				$visitor = db_get_row("SELECT name FROM ?:lh_sessions WHERE session_id = ?i", $from_id);
				$texts[$k]['name'] = $visitor['name'];
				$texts[$k++]['text'] = $row['text'];
			}
			$session_ids[] = $id;

			if (!array_key_exists($id, $session_info)) {
				$session_info[$id] = array(
					'min_date' => $date,
					'max_date' => $date,
					'num' => 0,
					'operator' => $op_id,
				);
			}
			if ($session_info[$id]['min_date'] > $date) {
				$session_info[$id]['min_date'] = $date;
			}
			if ($session_info[$id]['max_date'] < $date) {
				$session_info[$id]['max_date'] = $date;
			}
			$session_info[$id]['num'] += 1;
		}
		$session_ids = array_unique($session_ids);

		// Generating persons from chat sessions
		if ($session_ids) {
			$session_data = db_get_array("SELECT session_id AS id, name, ip FROM ?:lh_sessions WHERE session_id IN (?n)", $session_ids);

			foreach ($session_data as $row) {
				$operator_info = $operators_assoc[$session_info[$id]['operator']];
				$persons[] = array(
					'id' => $row['id'],
					'type' => 'chat',
					'name' => $row['name'],
					'min_date' => $session_info[$id]['min_date'],
					'max_date' => $session_info[$id]['max_date'],
					'num' => $session_info[$id]['num'],
					'operator_id' => $session_info[$id]['operator'],
					'operator_name' => "$operator_info[login] ($operator_info[name])",
					'operator_name_only' => "$operator_info[name]",
					'ip' => $row['ip'],
					'from_name' => '',
				);
			}

			$name_persons = Array();
			if($params['name']) {
				foreach ($persons as $row) {
					if($row['name'] == $params['name']) {
						$name_persons[] = $row;
					}
				}
				$persons = $name_persons;
			}
		}

		// search part in WHERE
		$_where_search = 'WHERE 1';
		$_where_search .= empty($where_search) ? '' : ' AND '. str_replace('{search_field}', "?:lh_visitors_notes.note", $where_search);
		$_where_search .= empty($session_ids) ? '' : db_quote(" AND NOT(?:lh_visitors_notes.visitor_id IN (?n))", $session_ids);
		$_where_search .= $params['operator'] == 'all' ? '' : " AND ?:lh_visitors_notes.operator_id='$params[operator]'";
		$_where_search .= empty($params['time_from']) || empty($params['time_to']) ? '' : " AND ?:lh_visitors_notes.date >= '$params[time_from]' AND ?:lh_visitors_notes.date < '$params[time_to]'";

		$note_visitors = db_get_array("
			SELECT ?:lh_visitors_notes.visitor_id AS id, MIN(?:lh_visitors_notes.date) as min_date, MAX(?:lh_visitors_notes.date) as max_date, IF(?:lh_sessions.name, ?:lh_sessions.name, CONCAT('" . fn_get_lang_var('anonimous') . " (', INET_NTOA(?:lh_visitors.ip), ')')) AS name, ?:lh_visitors_notes.operator_id, ?:lh_operators.name AS operator_name_only, CONCAT(?:lh_operators.login, ' (', ?:lh_operators.name, ')') AS operator_name, ?:lh_visitors.ip
			FROM ?:lh_visitors_notes
			LEFT JOIN ?:lh_visitors ON ?:lh_visitors_notes.visitor_id=?:lh_visitors.visitor_id
			LEFT JOIN ?:lh_sessions ON ?:lh_visitors.ip=?:lh_sessions.ip
			LEFT JOIN ?:lh_operators ON ?:lh_visitors_notes.operator_id=?:lh_operators.operator_id
			$_where_search
			GROUP BY ?:lh_visitors_notes.visitor_id");
		if (empty($note_visitors)) {
			$note_visitors = array();
		}
		foreach ($note_visitors as $k => $nv) {
			$note_visitors[$k]['type'] = 'chat';
			$note_visitors[$k]['selected_section'] = 'notes';
			$note_visitors[$k]['num'] = '0';
			$persons[] = $note_visitors[$k];
		}
	}

	if ($params['group'] == 'all' || $params['group'] == 'operator') {
		// WHERE part of query
		if ($params['operator'] != 'all') {
			$where_operators .= "(direction = '" . LH_DIRECTION_OPERATOR_OPERATOR . "' AND (from_id = '$params[operator]' OR to_id = '$params[operator]'))";
		} else {
			$where_operators .= "(direction = '" . LH_DIRECTION_OPERATOR_OPERATOR . "')";
		}
		// search part in WHERE
		if ($where_search) {
			$search_query = str_replace('{search_field}', 'text', $where_search);
			$where_operators = "($where_operators) AND ($search_query)";
		}

		// Getting chat sessions
		$operators_data = db_get_array("SELECT from_id, to_id, date, text, direction FROM ?:lh_messages WHERE $where_operators ORDER BY date");
		$operators_ids = array();
		$operators_info = array();
		foreach ($operators_data as $row) {
			list($from_id, $to_id, $date) = array($row['from_id'], $row['to_id'], $row['date']);

			if ($params['operator'] != 'all') {
				$id = ($from_id == $params['operator']) ? $from_id : $to_id;
				$operators_ids[] = $id;

				$ids = array($id);
				$op_ids = array(($from_id == $params['operator']) ? $to_id : $from_id);
			} else {
				$operators_ids[] = $from_id;
				$operators_ids[] = $to_id;

				$ids = array($from_id, $to_id);
				$op_ids = array($to_id, $from_id);
			}

			if($params['q']) {
				$from_id = $row['from_id'];
				$to_id = $row['to_id'];
				$operators_name = db_get_row("SELECT login, name FROM ?:lh_operators WHERE operator_id = ?i", $from_id);
				$operators_to = db_get_row("SELECT login, name FROM ?:lh_operators WHERE operator_id = ?i", $to_id);
				$texts[$k]['name'] = $operators_to['login'].' ('.$operators_to['name'].')';
				$texts[$k]['operator_name'] = $operators_name['login'].' ('.$operators_name['name'].')';
				$texts[$k++]['text'] = $row['text'];
			}

			foreach ($ids as $count => $id) {
				if (!array_key_exists($id, $operators_info)) {
					$operators_info[$id] = array(
						'min_date' => $date,
						'max_date' => $date,
						'num' => 0,
						'operator' => $op_ids[$count],
					);
				}
				if ($operators_info[$id]['min_date'] > $date) {
					$operators_info[$id]['min_date'] = $date;
				}
				if ($operators_info[$id]['max_date'] < $date) {
					$operators_info[$id]['max_date'] = $date;
				}
				$operators_info[$id]['num'] += 1;
			}
		}
		$operators_ids = array_unique($operators_ids);

		// Generating persons from operator sessions
		if ($operators_ids) {
			foreach ($operators_ids as $id) {
				$operator_info = $operators_assoc[$id];
				$other_operator_info = $operators_assoc[$operators_info[$id]['operator']];
				$persons[] = array(
					'id' => $operators_info[$id]['operator'],
					'type' => 'operator',
					'name' => "$other_operator_info[login] ($other_operator_info[name])",
					'name_only' => "$other_operator_info[name]",
					'min_date' => $operators_info[$id]['min_date'],
					'max_date' => $operators_info[$id]['max_date'],
					'num' => $operators_info[$id]['num'],
					'operator_id' => $id,
					'operator_name' => "$operator_info[login] ($operator_info[name])",
					'operator_name_only' => "$operator_info[name]",
					'from_name' => '',
				);
			}

			if($params['name']) {
				$name_persons = Array();
				foreach ($persons as $row) {
					if($row['name_only'] == $params['name']) {
						$name_persons[] = $row;
					}				}
				$persons = $name_persons;
			}
		}
	}

	if ($params['group'] == 'all' || $params['group'] == 'message') {
		// WHERE part of query
		if ($params['operator'] != 'all') {
			$where_messages .= "(operator_id = '$params[operator]' AND replied = '1')";
		} else {
			$where_messages .= "(replied = '1')";
		}
		// search part in WHERE
		if ($where_search) {
			$search_query = array(str_replace('{search_field}', 'message', $where_search), str_replace('{search_field}', 'reply', $where_search));
			$where_messages = "($where_messages) AND (($search_query[0]) OR ($search_query[1]))";
		}

		// Getting left messages
		$left_messages = db_get_array("SELECT message_id AS id, from_name, from_mail, date, reply_date, operator_id, message FROM ?:lh_left_messages WHERE $where_messages ORDER BY date");

		// Generating persons from left messages
		$name_persons = Array();
		if ($left_messages) {
			foreach ($left_messages as $row) {
				$operator_info = $operators_assoc[$row['operator_id']];
				$persons[] = array(
					'id' => $row['id'],
					'type' => 'message',
					'name' => "$row[from_name] ($row[from_mail])",
					'from_name' => "$row[from_name]",
					'from_mail' => "$row[from_mail]",
					'min_date' => $row['date'],
					'max_date' => $row['reply_date'],
					'num' => 2,
					'operator_id' => $row['operator_id'],
					'operator_name' => "$operator_info[login] ($operator_info[name])",
					'operator_name_only' => "$operator_info[name]",
				);

				if($params['q']) {
					$from_id = $row['operator_id'];
					$texts[$k]['operator_name'] = db_get_field("SELECT CONCAT(login,' (', name, ')') FROM ?:lh_operators WHERE operator_id = ?i", $from_id);
					$texts[$k]['name'] = "$row[from_name] ($row[from_mail])";
					$texts[$k++]['text'] = $row['message'];
				}
			}

			if($params['name']) {
				$name_persons = Array();
				foreach ($persons as $row) {
					if($row['from_name'] == $params['name']) {
						$name_persons[] = $row;
					}				}
				$persons = $name_persons;
			}

			if($params['email']) {
				$name_persons = Array();
				foreach($persons as $row) {
					if($row['from_mail'] == $params['email']) {
						$name_persons[] = $row;
					}
				}
				$persons = $name_persons;
			}
		}
	}

	// Sorting stuff
	if (empty($sort_order)) {
		$sort_order = 'asc';
	}
	if (empty($sort_by)) {
		$sort_by = 'min_date';
	}
	$view->assign('sort_order', ($sort_order == 'asc') ? 'desc' : 'asc');
	$view->assign('sort_by', $sort_by);

	// Appling sorting
	$column = array();
	foreach ($persons as $value) {
		$column[] = $value[$sort_by];
	}
	array_multisort($column, ($sort_order == 'desc') ? SORT_DESC : SORT_ASC, $persons);

	// pagination
	$items_per_page = Registry::get('settings.Appearance.admin_elements_per_page');
	$total_items = count($persons);
	$limit = fn_paginate($params['page'], $total_items, $items_per_page);  //FIXME
	if ($limit) {
		$offset = ($params['page'] - 1) * $items_per_page;
		$persons = array_slice($persons, $offset, $items_per_page);
	}

	$view->assign('texts', array_values($texts));
	$view->assign('persons', $persons);
	$view->assign('search', $params);

	$view->assign('operators', array_values($operators_assoc));
	
	fn_live_help_generate_sections('chat_history');
}

//
// 'View' page
//
if ($mode == 'view') {

	$selected_section = empty($_REQUEST['selected_section']) ? '' : $_REQUEST['selected_section'];
	$operator = empty($_REQUEST['operator']) ? '' : $_REQUEST['operator'];
	$page = empty($_REQUEST['page']) ? 1 : $_REQUEST['page'];

	$suffix = '';
	if (!empty($_REQUEST['period']) && $_REQUEST['period'] != 'A') {
		list($_REQUEST['time_from'], $_REQUEST['time_to']) = fn_create_periods($_REQUEST);

		$suffix = "&period=$_REQUEST[period]&from_Month=$_REQUEST[from_Month]&from_Day=$_REQUEST[from_Day]&from_Year=$_REQUEST[from_Year]&to_Month=$_REQUEST[to_Month]&to_Day=$_REQUEST[to_Day]&to_Year=$_REQUEST[to_Year]";
	}

	if ($selected_section == 'events_log' || $selected_section == 'notes') {

		$report_id = ($selected_section == 'events_log') ? 1 : 3; //FIXME!!! Very bad code


		// Getting visitor name and ip
		$ip = db_get_field("SELECT ip FROM ?:lh_visitors WHERE visitor_id = ?i", $_REQUEST['id']);
		if (!$ip) {
			return;
		}
		$name = db_get_field("SELECT name FROM ?:lh_sessions WHERE ip = ?s", $ip);
		$ip = long2ip($ip);
		if ($name) {
			$visitor_name = "$name ($ip)";
		} else {
			$visitor_name = $ip;
		}


		$report_id = empty($_REQUEST['report_id']) ? 1 : $_REQUEST['report_id'];

		$_SESSION['lh_previous_report_id'] = $report_id;

		// Generating of tabs
		$page_names = array(
			fn_get_lang_var('lh_visitor_events_log'),
			fn_get_lang_var('lh_visit_history'),
			fn_get_lang_var('lh_operator_notes'),
		);
		// [Page sections]
		$view->assign('selected_section', $report_id);
		// [/Page sections]

		$period = empty($_REQUEST['period']) ? 'A' : $_REQUEST['period'];

		$where = "visitor_id = '$_REQUEST[id]'";

		// Setting period
		if (!empty($_SESSION['lh_report_visitor_info'])) {
			extract($_SESSION['lh_report_visitor_info']);
		}

		if (!empty($_REQUEST['time_from']) && !empty($_REQUEST['time_to'])) {
			$where .= " AND (date >= '$_REQUEST[time_from]' AND date < '$_REQUEST[time_to]')";
		}

		// Sorting stuff
		$sort_order = empty($_REQUEST['sort_order']) ? 'desc' : $_REQUEST['sort_order'];
		$sort_by = empty($_REQUEST['sort_by']) ? 'date' : $_REQUEST['sort_by'];


		$items_per_page = Registry::get('settings.Appearance.admin_elements_per_page');

		if ($report_id == 1) {
			// Description of columns
			$columns = array(
				array('name' => 'date', 'lang' => 'date', 'type' => 'date'),
				array('name' => 'event', 'lang' => 'lh_event', 'type' => 'event'),
			);

			// pagination
			$items_per_page = Registry::get('settings.Appearance.admin_elements_per_page');
			$total_items = db_get_field("SELECT COUNT(*) FROM ?:lh_visitors_log WHERE $where");
			$data = array();
			if ($total_items) {
				$limit = fn_paginate($_REQUEST['page'], $total_items, $items_per_page);
				$data = db_get_array("SELECT date, event, operator_name AS operator FROM ?:lh_visitors_log WHERE $where ORDER BY $sort_by $sort_order $limit");
			}

			// events key to lang dictionary
			$event_to_lang = array(
				LH_LOG_CHAT_STARTED => 'lh_event_chat_started',
				LH_LOG_OPERATOR_JOINED => 'lh_event_operator_joined',
				LH_LOG_SENT_INVITATION => 'lh_event_invitation_sent',
				LH_LOG_ACCEPTED_INVITATION => 'lh_event_accepted_invitation',
				LH_LOG_DECLINED_INVITATION => 'lh_event_declined_invitation',
			);

			$view->assign('event_to_lang', $event_to_lang);

			$view->assign('sort_order', ($sort_order == 'asc') ? 'desc' : 'asc');
			$view->assign('sort_by', $sort_by);

			$view->assign('data', $data);
			$view->assign('columns', $columns);

		} elseif ($report_id == 3) {
			// Description of columns
			$columns = array(
				array('name' => 'date', 'lang' => 'date', 'type' => 'date'),
				array('name' => 'operator', 'lang' => 'lh_operator', 'type' => 'operator'),
				array('name' => 'note', 'lang' => 'note', 'type' => ''),
			);

			// pagination
			$total_items = db_get_field("SELECT COUNT(*) FROM ?:lh_visitors_notes WHERE $where");
			$data = array();
			if ($total_items) {
				$limit = fn_paginate(@$_REQUEST['page'], $total_items, $items_per_page);
				$data = db_get_array("SELECT date, operator_name AS operator, note FROM ?:lh_visitors_notes WHERE $where ORDER BY $sort_by $sort_order $limit");
			}
			$view->assign('sort_order', ($sort_order == 'asc') ? 'desc' : 'asc');
			$view->assign('sort_by', $sort_by);
			$view->assign('data', $data);

			$view->assign('columns', $columns);
		}

		$view->assign('period', $period);
		$view->assign('visitor_id', $_REQUEST['id']);

	} else {

		// Chat
		$selected_section = 'chat';

		$type = (empty($_REQUEST['type']) || !in_array($_REQUEST['type'], array('chat', 'message', 'operator'))) ? 'chat' : $_REQUEST['type'];

		$operator_name = db_get_field("SELECT CONCAT(login, ' (', name, ')') FROM ?:lh_operators WHERE operator_id = ?i", $_REQUEST['operator']);
		$operator_name = empty($operator_name) ? fn_get_lang_var('none') : $operator_name;

		if ($type == 'chat') {
			$person_name = db_get_field("SELECT name FROM ?:lh_sessions WHERE session_id = ?i", $_REQUEST['id']);
		} elseif ($type == 'operator') {
			$person_name = db_get_field("SELECT CONCAT(login, ' (', name, ')') FROM ?:lh_operators WHERE operator_id = ?i", $_REQUEST['id']);
		} elseif ($type == 'message') {
			$person_name = db_get_field("SELECT from_name FROM ?:lh_left_messages WHERE message_id = ?i", $_REQUEST['id']);
		}
		if (!$person_name) {
			$person_name = "-";
		}

		// period
		$where = '';
		if ($type != 'message') {

			if (!empty($_REQUEST['time_from']) && !empty($_REQUEST['time_to'])) {
				$where = db_quote("(date >= ?i AND date < ?i) AND ", $_REQUEST['time_from'], $_REQUEST['time_to']);
			}

		}

		$highlight_assoc = array();
		if (isset($highlight)) {
			$highlight_words = explode(' ', strip_tags($highlight));
			foreach ($highlight_words as $value) {
				$highlight_assoc[$value] = "<span style='background-color: lightgreen;'>$value</span>";
			}
		}

		$messages = array();
		if ($type == 'chat') {
			// Fetching messages
			$total = db_get_field("SELECT count(*) FROM ?:lh_messages WHERE $where ((direction = '" . LH_DIRECTION_VISITOR_OPERATOR ."' AND from_id = '$_REQUEST[id]' AND to_id = '$operator') OR (direction = '" . LH_DIRECTION_OPERATOR_VISITOR . "' AND from_id = '$operator' AND to_id = '$_REQUEST[id]'))");
			$limit = fn_paginate(@$_REQUEST['page'], $total, Registry::get('settings.Appearance.admin_elements_per_page'));
			$messages = db_get_array("SELECT direction, from_id, to_id, date, text AS message FROM ?:lh_messages WHERE $where ((direction = '" . LH_DIRECTION_VISITOR_OPERATOR ."' AND from_id = '$_REQUEST[id]' AND to_id = '$operator') OR (direction = '" . LH_DIRECTION_OPERATOR_VISITOR . "' AND from_id = '$operator' AND to_id = '$_REQUEST[id]')) $limit");
			foreach ($messages as $key => $value) {
				$messages[$key]['from_name'] = $value['direction'] == LH_DIRECTION_VISITOR_OPERATOR ? $person_name : $operator_name;
				$messages[$key]['direction'] = $value['direction'] == LH_DIRECTION_VISITOR_OPERATOR ? 0 : 1;

				fn_convert_message($messages[$key], $highlight_assoc);
			}
		} elseif ($type == 'operator') {
			// Fetching messages
			$total = db_get_field("SELECT count(*) FROM ?:lh_messages WHERE $where (direction = '" . LH_DIRECTION_OPERATOR_OPERATOR . "' AND ((from_id = '$_REQUEST[id]' AND to_id = '$operator') OR (to_id = '$_REQUEST[id]' AND from_id = '$operator')))");
			$limit = fn_paginate(@$_REQUEST['page'], $total, Registry::get('settings.Appearance.admin_elements_per_page'));
			$messages = db_get_array("SELECT from_id, to_id, date, text AS message FROM ?:lh_messages WHERE $where (direction = '" . LH_DIRECTION_OPERATOR_OPERATOR . "' AND ((from_id = '$_REQUEST[id]' AND to_id = '$operator') OR (to_id = '$_REQUEST[id]' AND from_id = '$operator'))) $limit");
			foreach ($messages as $key => $value) {
				$messages[$key]['from_name'] = ($value['from_id'] == $_REQUEST['id']) ? $person_name : $operator_name;
				$messages[$key]['direction'] = ($value['from_id'] == $_REQUEST['id']) ? 0 : 1;

				fn_convert_message($messages[$key], $highlight_assoc);
			}
		} elseif ($type == 'message') {
			$result = db_get_row("SELECT date, reply_date, from_name, from_mail, subject, message, reply FROM ?:lh_left_messages WHERE message_id = ?i", $_REQUEST['id']);

			$result['message'] = str_replace(array_keys($highlight_assoc), array_values($highlight_assoc), $result['message']);
			$result['reply'] = str_replace(array_keys($highlight_assoc), array_values($highlight_assoc), $result['reply']);

			$messages[0] = array(
				'date' => $result['date'],
				'from_name' => "$result[from_name] (<a href='mailto:$result[from_mail]'>$result[from_mail]</a>)",
				'direction' => 0,
				'message' => $result['message'],
				'subject' => $result['subject'],
				'type' => 'mail',
			);
			$messages[1] = array(
				'date' => $result['reply_date'],
				'from_name' => $operator_name,
				'direction' => 1,
				'message' => $result['reply'],
				'type' => 'reply',
			);
		}

		$view->assign('messages', $messages);
	}

	// [Main breadcrumbs]
	fn_add_breadcrumb(fn_get_lang_var('lh_chat_history'), "chat_history.manage");

	switch ($selected_section) {
		case 'events_log':
			break;
		case 'notes':
			break;
		default:
			$replaces = array(
				'['. ($type == 'operator' ? 'name1' : 'operator') .']' => $operator_name,
				'['. ($type == 'operator' ? 'name2' : 'name') .']' => $person_name,
			);
			if ($type == 'chat') {
				$crumb_title = str_replace(array_keys($replaces), array_values($replaces), fn_get_lang_var('lh_chat_session_between_visitor_and_operator'));
			} elseif ($type == 'operator') {
				$crumb_title = str_replace(array_keys($replaces), array_values($replaces), fn_get_lang_var('lh_chat_session_between_operators'));
			} elseif ($type == 'message') {
				$crumb_title = str_replace(array_keys($replaces), array_values($replaces), fn_get_lang_var('lh_left_message_of_visitor_replied_by'));
			}

			fn_add_breadcrumb($crumb_title, "chat_history.view?type=$type&id=$_REQUEST[id]&operator=$operator&selected_section=$selected_section");
			break;
	}
	// [/Main breadcrumbs]

	Registry::set('navigation.tabs', array (
		'chat' => array (
			'href' => "chat_history.view?type=$type&id=$_REQUEST[id]&operator=$operator&selected_section=chat$suffix",
			'title' => fn_get_lang_var('chat')
		)
	));

	if ($type == 'chat') {
		Registry::set('navigation.tabs.events_log', array (
			'href' => "chat_history.view?type=$type&id=$_REQUEST[id]&operator=$operator&selected_section=events_log$suffix",
			'title' => fn_get_lang_var('lh_visitor_events_log')
		));
		Registry::set('navigation.tabs.notes', array (
			'href' => "chat_history.view?type=$type&id=$_REQUEST[id]&operator=$operator&selected_section=notes$suffix",
			'title' => fn_get_lang_var('notes')
		));
	}

	$tpl_vars = array(
		'type' => $type,
		'id' => $_REQUEST['id'],
		'operator' => $operator,
		'selected_section' => $selected_section,
		'page' => $page,
	);
	foreach ($tpl_vars as $name => $val) {
		$view->assign($name, $val);
	}
}


/*function fn_get_chat_history($params)
{
	// Init filter
	$params = fn_init_view('chat_history', $params);

	// Set default values to input params
	$default_params = array (
		'operator' => 'all',
		'match' => 'any',
		'group' => 'all',
		'page' => 1,
	);

	$params = array_merge($default_params, $params);

	$join = $condition = $group_by = '';

	// Filter by periods
	if (!empty($params['period']) && $params['period'] != 'A') {
		list($params['time_from'], $params['time_to']) = fn_create_periods($params);

		$condition .= db_quote(" AND (date >= ?i AND date <= ?i)", $params['time_from'], $params['time_to']);
	}

	// Filter by message text
	if ($params['q']) {
		$words = explode(' ', $params['q']);
		if ($params['match'] == 'any') {
			$where_search = array();
			foreach ($words as $word) {
				$where_search[] = db_quote("?:lh_messages.text LIKE ?l", "%$word%");
			}
			$condition .= ' AND (' . implode(' OR ', $where_search) . ')';
		} elseif ($params['match'] == 'all') {
			$where_search = array();
			foreach ($words as $word) {
				$where_search[] = db_quote("?:lh_messages.text LIKE ?l", "%$word%");
			}

			$condition .= ' AND (' . implode(' OR ', $where_search) . ')';
		} elseif ($params['match'] == 'exact') {

			$condition .= db_quote("?:lh_messages.text LIKE ?l", $params['q']);
		}
	}

	// Filter by operator ID
	if (!empty($params['operator_id'])) {
		$condition .= db_quote("(?:lh_messages.direction = '" . LH_DIRECTION_VISITOR_OPERATOR . "' AND ?:lh_messages.to_id = ?i) OR (?:lh_messages.direction = '" . LH_DIRECTION_OPERATOR_VISITOR . "' AND ?:lh_messages.from_id = ?i)", $params['operator_id'], $params['operator_id']);
	}

	// Filter by visitor name
	if (!empty($params['name'])) {
		$condition .= db_quote("?:lh_visitors.assigned_name LIKE ?l", $params['name']);
	}

	// Filter by email (in left messages)
	if (!empty($params['email'])) {
		$condition .= db_quote("?:lh_left_messages.email LIKE ?l", $params['email']);
	}

	// Search in chats
	if ($params['group'] == 'chat') {

	// Search in left messages
	} elseif ($params['group'] == 'messages') {

	// Search in operators chats
	} elseif ($params['group'] == 'operator') {
	}
}*/

?>
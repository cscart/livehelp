<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */


if ( !defined('AREA') )	{ die('Access denied');	}

/* POST data processing */
if ($_SERVER['REQUEST_METHOD'] == 'POST') {

	$suffix = '';
	
	//
	// Update operators
	//
	if ($mode == 'm_update') {
		// Update multiple pages data
		if (is_array($_REQUEST['operator_data'])) {
			foreach ($_REQUEST['operator_data'] as $k => $v) {
				db_query("UPDATE ?:lh_operators SET ?u WHERE operator_id = ?i", $v, $k);
			}
		}

		$suffix = '.manage';
	}
	
	//
	// Add new operator
	//
	if ($mode == 'add') {
		$operator_data = $_REQUEST['operator_data'];

		// Check if user already exists
		$is_exist = db_get_field("SELECT login FROM ?:lh_operators WHERE login = ?s", $operator_data['login']);
		if ($is_exist) {
			fn_save_post_data();
			fn_set_notification('E', fn_get_lang_var('error'), fn_get_lang_var('error_user_exists'));

			return array(CONTROLLER_STATUS_REDIRECT, "operators.manage");
		}
		
		// Check the passwords
		$operator_data['password1'] = (strlen(trim($operator_data['password1'])) == 0) ? '' : $operator_data['password1'];
		$operator_data['password2'] = (strlen(trim($operator_data['password2'])) == 0) ? '' : $operator_data['password2'];
		
		if (!empty($operator_data['password1']) && !empty($operator_data['password2']) && $operator_data['password1'] == $operator_data['password2']) {
			$operator_data['password'] = md5($operator_data['password1']);
		} elseif ($operator_data['password1'] != $operator_data['password2']) {
			fn_save_post_data();
			fn_set_notification('E', fn_get_lang_var('error'), fn_get_lang_var('error_passwords_dont_match'));

			return array(CONTROLLER_STATUS_REDIRECT, "operators.manage");
		}
		
		// Adding new operator
		$operator_data['register_time'] = TIME;
		db_query("INSERT INTO ?:lh_operators ?e", $operator_data);

		$suffix = '.manage';
	}
	
	//
	// Add new operator
	//
	if ($mode == 'update') {
		$operator_data = $_REQUEST['operator_data'];

		// Check the passwords
		$operator_data['password1'] = (strlen(trim($operator_data['password1'])) == 0) ? '' : $operator_data['password1'];
		$operator_data['password2'] = (strlen(trim($operator_data['password2'])) == 0) ? '' : $operator_data['password2'];
		
		if (!empty($operator_data['password1']) && !empty($operator_data['password2']) && $operator_data['password1'] == $operator_data['password2']) {
			$operator_data['password'] = md5($operator_data['password1']);

		} elseif ($operator_data['password1'] != $operator_data['password2']) {
			fn_save_post_data();
			fn_set_notification('E', fn_get_lang_var('error'), fn_get_lang_var('error_passwords_dont_match'));

			return array(CONTROLLER_STATUS_REDIRECT, "operators.manage");
		}

		// Updating page of operator
		db_query("UPDATE ?:lh_operators SET ?u WHERE operator_id = ?i", $operator_data, $operator_data['operator_id']);

		$suffix = '.update?operator_id=' . $operator_data['operator_id'];
	}

	//
	// Processing deleting of multiple page elements
	//
	if ($mode == 'delete') {

		// Deleting all according to operator data from database
		db_query("DELETE FROM ?:lh_left_messages WHERE operator_id IN (?n)", $_REQUEST['operator_ids']);
		db_query("DELETE FROM ?:lh_messages WHERE (direction = ?s AND to_id IN (?n)) OR (direction = ?s AND from_id IN (?n)) OR (direction = ?s AND (from_id IN (?n) OR to_id IN (?n)))", LH_DIRECTION_VISITOR_OPERATOR, $_REQUEST['operator_ids'], LH_DIRECTION_OPERATOR_VISITOR, $_REQUEST['operator_ids'], LH_DIRECTION_OPERATOR_OPERATOR, $_REQUEST['operator_ids'], $_REQUEST['operator_ids']);
		db_query("DELETE FROM ?:lh_messages_map WHERE self_id IN (?n)", $_REQUEST['operator_ids']);
		db_query("DELETE FROM ?:lh_type_notify WHERE (direction = 0 AND to_id IN (?n)) OR (direction = 1 AND self_id IN (?n)) OR (direction = 2 AND (self_id IN (?n) OR to_id IN (?n)))", $_REQUEST['operator_ids'], $_REQUEST['operator_ids'], $_REQUEST['operator_ids'], $_REQUEST['operator_ids']);
		db_query('UPDATE ?:lh_visitors SET ?u WHERE operator_id IN (?n)', array('operator_id' => 0), $_REQUEST['operator_ids']);
		db_query("DELETE FROM ?:lh_visitors_log WHERE operator_id IN (?n)", $_REQUEST['operator_ids']);
		db_query("DELETE FROM ?:lh_visitors_notes WHERE operator_id IN (?n)", $_REQUEST['operator_ids']);
		
		db_query("DELETE FROM ?:lh_operators WHERE operator_id IN (?n)", $_REQUEST['operator_ids']);
		
		fn_set_notification('N', fn_get_lang_var('congratulations'), fn_get_lang_var('lh_operators_have_been_deleted'));

		$suffix = '.manage';
	}

	return array(CONTROLLER_STATUS_OK, "operators$suffix");
}

if ($mode == 'manage') {

	// sorting stuff
	$sort_order = empty($_REQUEST['sort_order']) ? 'asc' : $_REQUEST['sort_order'];
	$sort_by = empty($_REQUEST['sort_by']) ? 'login' : $_REQUEST['sort_by'];

	$sorting = "$sort_by $sort_order";
	
	// pagination
	$items_per_page = Registry::get('settings.Appearance.admin_elements_per_page');
	$total_items = db_get_field("SELECT COUNT(*) FROM ?:lh_operators");
	$limit = fn_paginate(@$_REQUEST['page'], $total_items, $items_per_page); // FIXME
	
	$operators = db_get_array("SELECT operator_id, login, name, register_time, status FROM ?:lh_operators ORDER BY $sorting $limit");
	
	$view->assign('sort_order', ($sort_order == 'asc') ? 'desc' : 'asc');
	$view->assign('sort_by', $sort_by);
	$view->assign('operators', $operators);
	
	fn_live_help_generate_sections('operators');

//
// Add operator
//
} elseif ($mode == 'add') {

	fn_add_breadcrumb(fn_get_lang_var('lh_operators'), "operators.manage");

	$operator_data = array();
	if (!empty($_SESSION['saved_post_data'])) {
		$operator_data = $_SESSION['saved_post_data']['operator_data'];
	}
	unset($_SESSION['saved_post_data']);
	
	$view->assign('operator_data', $operator_data);

//
// Update operator
//
} elseif ($mode == 'update') {
	fn_add_breadcrumb(fn_get_lang_var('lh_operators'), "operators.manage");

	if (empty($_SESSION['saved_post_data'])) {
		$operator_data = db_get_row("SELECT operator_id, login, name, status FROM ?:lh_operators WHERE operator_id = ?i", $_REQUEST['operator_id']);
		
		if (empty($operator_data)) {
			return array(CONTROLLER_STATUS_NO_PAGE);
		}

		$operator_data['password1'] = $operator_data['password2'] = str_repeat(' ', 12);
	} else {
		$operator_data = $_SESSION['saved_post_data']['operator_data'];
		unset($_SESSION['saved_post_data']);
	}
	
	$view->assign('operator_data', $operator_data);
}

?>
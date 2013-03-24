<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */


if ( !defined('AREA') ) { die('Access denied'); }

$schema['chat_history'] = array (
	'permissions' => 'manage_livehelp',
);

$schema['operators'] = array (
	'permissions' => 'manage_livehelp',
);

$schema['tools']['modes']['update_status']['param_permissions']['table_names']['lh_operators'] = 'manage_livehelp';

?>
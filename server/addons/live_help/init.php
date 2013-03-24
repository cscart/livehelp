<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */

if ( !defined('AREA') ) { die('Access denied'); }

fn_register_hooks(
	'init_secure_controllers',
	'fill_auth'
);

?>
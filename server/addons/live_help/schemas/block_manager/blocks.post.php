<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */

if ( !defined('AREA') ) { die('Access denied'); }

$schema['live_help'] = array (
	'content' => array(
		'live_help_opts' => array(
			'type' => 'function',
			'function' => array('fn_get_livehelp_options')
		),
	),
	'templates' => array(
		'addons/live_help/blocks/livehelp.tpl' => array(),
	),
	'wrappers' => 'blocks/wrappers',
);
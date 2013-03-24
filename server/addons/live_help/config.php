<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */

if ( !defined('AREA') ) { die('Access denied'); }

//
// Static configuration
//

Registry::set('livehelp_config', array(
	'freshness_time' => 5,
	'freshness_time_operator' => 15,
	'freshness_time_visitor' => 6,
	'chat_interval' => 3000,
	'obsolete_time' => 3,
	'visitor_data_clean_period' => 86400,
));

//
// Directories
//
define('DIR_LH_UPLOADS', DIR_ROOT . '/var/lh_uploads');

//
// Constants
//

// Visitor status
define('LH_STATUS_IDLE', 0);
define('LH_STATUS_WAITING', 1);
define('LH_STATUS_CHATTING', 2);
define('LH_STATUS_REQUEST_SENT', 3);
define('LH_STATUS_DECLINED', 4);

// Message directions
define('LH_DIRECTION_VISITOR_OPERATOR', 0);
define('LH_DIRECTION_OPERATOR_VISITOR', 1);
define('LH_DIRECTION_OPERATOR_OPERATOR', 2);

// Last messages map directions
define('LH_DIRECTION_MAP_SESSIONS', 0);
define('LH_DIRECTION_MAP_OPERATORS', 1);

// Typing notifications
define('LH_TYPING_ON', 1);
define('LH_TYPING_OFF', -1);

// Visitor logging events
define('LH_LOG_CHAT_STARTED', 1);
define('LH_LOG_OPERATOR_JOINED', 2);
define('LH_LOG_SENT_INVITATION', 3);
define('LH_LOG_ACCEPTED_INVITATION', 4);
define('LH_LOG_DECLINED_INVITATION', 5);
define('LH_LOG_PAGE_CHANGED', 11);

// Auth result
define('LH_AUTH_RESULT_OK', 1);
define('LH_AUTH_RESULT_FAIL', 2);
define('LH_AUTH_RESULT_ACTIVE', 3);

?>
-- phpMyAdmin SQL Dump
-- version 4.7.4
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1:3306
-- Generation Time: 2019-05-12 02:55:09
-- 服务器版本： 5.7.19
-- PHP Version: 5.6.31

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
SET AUTOCOMMIT = 0;
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: `bangumi`
--

-- --------------------------------------------------------

--
-- 表的结构 `bgm_subjects`
--

DROP TABLE IF EXISTS `bgm_subjects`;
CREATE TABLE IF NOT EXISTS `bgm_subjects` (
  `subject_id` int(10) UNSIGNED NOT NULL,
  `url` text,
  `type` tinyint(3) UNSIGNED NOT NULL DEFAULT '0',
  `name` text,
  `name_cn` text,
  `summary` text,
  `eps` smallint(5) UNSIGNED NOT NULL DEFAULT '0',
  `air_date` date DEFAULT NULL,
  `air_weekday` tinyint(4) NOT NULL DEFAULT '0',
  `rating_num` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `rating_score` float UNSIGNED NOT NULL DEFAULT '0',
  `rank` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `image_file` text,
  `wish` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `collect` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `doing` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `on_hold` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `dropped` int(10) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`subject_id`),
  KEY `subject_id` (`subject_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- 表的结构 `bgm_users`
--

DROP TABLE IF EXISTS `bgm_users`;
CREATE TABLE IF NOT EXISTS `bgm_users` (
  `user_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `user_qq` bigint(20) UNSIGNED NOT NULL,
  `user_bangumi` int(10) UNSIGNED NOT NULL,
  `user_access_token` text NOT NULL,
  `user_refresh_token` text NOT NULL,
  `user_last_searched` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `dmhy_open` tinyint(1) NOT NULL DEFAULT '0',
  `dmhy_keyword` mediumtext,
  `dmhy_lastpubDate` datetime DEFAULT '1000-01-01 00:00:00',
  `dmhy_moe` tinyint(1) NOT NULL DEFAULT '0',
  `BgmCode_subject` int(10) NOT NULL DEFAULT '0',
  `BgmCode_search` int(10) NOT NULL DEFAULT '0',
  `BgmCode_user` int(10) NOT NULL DEFAULT '0',
  `BgmCode_up` int(10) NOT NULL DEFAULT '0',
  `BgmCode_collect` int(10) NOT NULL DEFAULT '0',
  `BgmCode_reg` int(10) NOT NULL DEFAULT '0',
  `BgmCode_help` int(10) NOT NULL DEFAULT '0',
  `BgmCode_tag` int(10) NOT NULL DEFAULT '0',
  `BgmCode_statis` int(10) NOT NULL DEFAULT '0',
  `BgmCode_unknow` int(10) NOT NULL DEFAULT '0',
  `BgmCode_Last_Date` datetime NOT NULL DEFAULT '1000-01-01 00:00:00',
  `TBgmCode_subject` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_search` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_user` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_up` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_collect` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_reg` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_help` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_tag` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_statis` int(10) NOT NULL DEFAULT '0',
  `TBgmCode_unknow` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`user_id`),
  UNIQUE KEY `bgm_id` (`user_id`),
  UNIQUE KEY `bgm_qq` (`user_qq`)
) ENGINE=InnoDB AUTO_INCREMENT=17 DEFAULT CHARSET=utf8mb4;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

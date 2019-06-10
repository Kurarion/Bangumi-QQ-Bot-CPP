-- phpMyAdmin SQL Dump
-- version 4.8.1
-- https://www.phpmyadmin.net/
--
-- Host: localhost
-- Generation Time: 2019-06-10 14:13:49
-- 服务器版本： 5.5.60-log
-- PHP Version: 5.6.36

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

CREATE TABLE `bgm_subjects` (
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
  `score_1` smallint(6) NOT NULL DEFAULT '0',
  `score_2` smallint(6) NOT NULL DEFAULT '0',
  `score_3` smallint(6) NOT NULL DEFAULT '0',
  `score_4` smallint(6) NOT NULL DEFAULT '0',
  `score_5` smallint(6) NOT NULL DEFAULT '0',
  `score_6` smallint(6) NOT NULL DEFAULT '0',
  `score_7` smallint(6) NOT NULL DEFAULT '0',
  `score_8` smallint(6) NOT NULL DEFAULT '0',
  `score_9` smallint(6) NOT NULL DEFAULT '0',
  `score_10` smallint(6) NOT NULL DEFAULT '0',
  `score_max` smallint(6) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

-- --------------------------------------------------------

--
-- 表的结构 `bgm_users`
--

CREATE TABLE `bgm_users` (
  `user_id` int(10) UNSIGNED NOT NULL,
  `user_qq` bigint(20) UNSIGNED NOT NULL,
  `user_bangumi` int(10) UNSIGNED NOT NULL,
  `user_access_token` text NOT NULL,
  `user_refresh_token` text NOT NULL,
  `user_last_searched` int(10) UNSIGNED NOT NULL DEFAULT '0',
  `dmhy_open` tinyint(1) NOT NULL DEFAULT '0',
  `dmhy_keyword` mediumtext,
  `dmhy_lastpubDate` datetime NOT NULL DEFAULT '1000-01-01 00:00:00',
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
  `TBgmCode_unknow` int(10) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb4;

--
-- Indexes for dumped tables
--

--
-- Indexes for table `bgm_subjects`
--
ALTER TABLE `bgm_subjects`
  ADD PRIMARY KEY (`subject_id`),
  ADD KEY `subject_id` (`subject_id`);

--
-- Indexes for table `bgm_users`
--
ALTER TABLE `bgm_users`
  ADD PRIMARY KEY (`user_id`),
  ADD UNIQUE KEY `bgm_id` (`user_id`),
  ADD UNIQUE KEY `bgm_qq` (`user_qq`);

--
-- 在导出的表使用AUTO_INCREMENT
--

--
-- 使用表AUTO_INCREMENT `bgm_users`
--
ALTER TABLE `bgm_users`
  MODIFY `user_id` int(10) UNSIGNED NOT NULL AUTO_INCREMENT;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;

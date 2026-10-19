-- MySQL dump 10.13  Distrib 8.4.3, for Win64 (x86_64)
--
-- Host: localhost    Database: acore_world
-- ------------------------------------------------------
-- Server version	8.4.3

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `vehicleseat_dbc`
--

DROP TABLE IF EXISTS `vehicleseat_dbc`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `vehicleseat_dbc` (
  `ID` int NOT NULL DEFAULT '0',
  `field1` float DEFAULT NULL,
  `field2` int DEFAULT '0',
  `field3` float DEFAULT NULL,
  `field4` float DEFAULT NULL,
  `field5` float DEFAULT NULL,
  `field6` float DEFAULT NULL,
  `field7` float DEFAULT NULL,
  `field8` float DEFAULT NULL,
  `field9` float DEFAULT NULL,
  `field10` float DEFAULT NULL,
  `field11` float DEFAULT NULL,
  `field12` int DEFAULT '0',
  `field13` int DEFAULT '0',
  `field14` int DEFAULT '0',
  `field15` int DEFAULT '0',
  `field16` int DEFAULT '0',
  `field17` int DEFAULT '0',
  `field18` int DEFAULT '0',
  `field19` float DEFAULT NULL,
  `field20` int DEFAULT '0',
  `field21` float DEFAULT NULL,
  `field22` float DEFAULT NULL,
  `field23` float DEFAULT NULL,
  `field24` float DEFAULT NULL,
  `field25` int DEFAULT '0',
  `field26` int DEFAULT '0',
  `field27` int DEFAULT '0',
  `field28` int DEFAULT '0',
  `field29` float DEFAULT NULL,
  `field30` float DEFAULT NULL,
  `field31` float DEFAULT NULL,
  `field32` int DEFAULT '0',
  `field33` int DEFAULT '0',
  `field34` int DEFAULT '0',
  `field35` int DEFAULT '0',
  `field36` int DEFAULT '0',
  `field37` int DEFAULT '0',
  `field38` int DEFAULT '0',
  `field39` float DEFAULT NULL,
  `field40` float DEFAULT NULL,
  `field41` int DEFAULT '0',
  `field42` int DEFAULT '0',
  `field43` float DEFAULT NULL,
  `field44` int DEFAULT '0',
  `field45` int DEFAULT '0',
  `field46` float DEFAULT NULL,
  `field47` float DEFAULT NULL,
  `field48` int DEFAULT '0',
  `field49` float DEFAULT NULL,
  `field50` int DEFAULT '0',
  `field51` int DEFAULT '0',
  `field52` float DEFAULT NULL,
  `field53` float DEFAULT NULL,
  `field54` int DEFAULT '0',
  `field55` int DEFAULT '0',
  `field56` int DEFAULT '0',
  `field57` int DEFAULT '0',
  PRIMARY KEY (`ID`),
  UNIQUE KEY `ID` (`ID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8mb3 COLLATE=utf8mb3_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `vehicleseat_dbc`
--

LOCK TABLES `vehicleseat_dbc` WRITE;
/*!40000 ALTER TABLE `vehicleseat_dbc` DISABLE KEYS */;
/*!40000 ALTER TABLE `vehicleseat_dbc` ENABLE KEYS */;
UNLOCK TABLES;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2026-10-19  5:16:13

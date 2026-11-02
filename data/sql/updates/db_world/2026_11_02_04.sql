-- Seat 3010 (vehicle kit 337): exit 3 yards behind the vehicle.
-- This is the seat used by the Scarlet Cannon (28833); the seat is also shared
-- with other vehicles using kit 337, including 33067.
DELETE FROM `vehicle_seat_addon` WHERE `SeatEntry`=3010;
INSERT INTO `vehicle_seat_addon`
(`SeatEntry`,`SeatOrientation`,`ExitParamX`,`ExitParamY`,`ExitParamZ`,`ExitParamO`,`ExitParamValue`)
VALUES
(3010,0,0,-3,0,0,1);

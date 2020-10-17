TRUNCATE "accounts"."account" CASCADE;

INSERT INTO "accounts"."account" VALUES
	(1, 'test', '$1$illarion$QPEdmswqVsgd4bDsj2ZeL/', 'test@dev.null', '2013-07-16 16:45:19.548574', '127.0.0.1' , 3, 5, 1, NULL, '0,1,2,3,4,5', '6,7,8', 'test', 100, 1, NULL, 0, 'metric', 'metric');

INSERT INTO "accounts"."account_groups" VALUES
	(1, 1, 1);

INSERT INTO "illarionserver"."chars" VALUES
	(1, 1, 0, NULL, NULL, NULL, '127.0.0.1', 30000, 0, 0, 0, 'Testserver One', '', ''),
	(1, 2, 0, NULL, NULL, NULL, '127.0.0.1', 30000000, 0, 0, 0, 'Testserver Two', '', '');

INSERT INTO "illarionserver"."player" VALUES
	(1, 3, 5, 1000, 0, 47, 10000, 80, 10000, 0, 0, 0, 14, 14, 16, 14, 7, 9, 5, 5, 13830, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1899272, 3645256, 0, 0, 255, 255, 255, 255, 255, 255),
	(2, 4, 5, 1000, 0, 47, 10000, 80, 10000, 0, 0, 0, 14, 14, 16, 14, 7, 9, 5, 5, 13830, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1899272, 3645256, 0, 0, 255, 255, 255, 255, 255, 255);

INSERT INTO "illarionserver"."gms" VALUES
	('Testserver', 1, 66562, 0),
	('Testserver', 2, 66562, 0);

INSERT INTO "illarionserver"."playerskills" VALUES
	(1, 0, 100, 0, 0, 0), (1, 1, 100, 0, 0, 0),
	(2, 0, 100, 0, 0, 0), (2, 1, 100, 0, 0, 0);

INSERT INTO "illarionserver"."playeritems" VALUES
	( 97, 1,  1, 0, 0, 9, 1, 555, 0),
	( 93, 1, 13, 0, 0, 9, 1, 555, 0),
	( 99, 1, 14, 0, 0, 9, 1, 555, 0),
	(100, 1, 15, 0, 0, 9, 1, 555, 0),
	(382, 1, 16, 0, 0, 9, 1, 555, 0),
	( 97, 2,  1, 0, 0, 9, 1, 555, 0),
	( 93, 2, 13, 0, 0, 9, 1, 555, 0),
	( 99, 2, 14, 0, 0, 9, 1, 555, 0),
	(100, 2, 15, 0, 0, 9, 1, 555, 0),
	(382, 2, 16, 0, 0, 9, 1, 555, 0);


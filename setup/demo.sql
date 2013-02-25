--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

SET search_path = accounts, pg_catalog;

--
-- Name: acc_group_seq; Type: SEQUENCE SET; Schema: accounts; Owner: demoserver
--

SELECT pg_catalog.setval('acc_group_seq', 1, false);


--
-- Name: acc_log_seq; Type: SEQUENCE SET; Schema: accounts; Owner: demoserver
--

SELECT pg_catalog.setval('acc_log_seq', 1, false);


--
-- Data for Name: account; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY account (acc_id, acc_login, acc_passwd, acc_email, acc_registerdate, acc_lastip, acc_state, acc_maxchars, acc_lang, acc_newmail, acc_racepermission, acc_applypermission, acc_name, acc_timeoffset, acc_dst, acc_lastseen, acc_recv_inact_mails, acc_length, acc_weight) WITH OIDS FROM stdin;
4445760	1	test	$1$illarion$mqk9WDaGFzlfDenjOq/Th0	test@test.test	2013-02-25 19:35:00	127.0.0.1	3	5	0		0,1,2,3,4,5	6,7,8	test	0	1	2013-02-25 19:35:00	0	metric	metric
\.


--
-- Data for Name: groups; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY groups (g_id, g_name_de, g_name_us, g_desc_de, g_desc_us, g_rights) FROM stdin;
\.


--
-- Data for Name: account_groups; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY account_groups (ag_id, ag_acc_id, ag_group_id) FROM stdin;
\.


--
-- Data for Name: account_log; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY account_log (al_id, al_user_id, al_gm_id, al_time, al_message, al_type) FROM stdin;
\.


--
-- Name: account_seq; Type: SEQUENCE SET; Schema: accounts; Owner: demoserver
--

SELECT pg_catalog.setval('account_seq', 1, false);


--
-- Data for Name: attribtemp; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY attribtemp (attr_id, attr_name_de, attr_name_us, attr_str, attr_dex, attr_agi, attr_con, attr_per, attr_int, attr_wil, attr_ess) FROM stdin;
\.


--
-- Data for Name: bad_ips; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY bad_ips (bip_ip) WITH OIDS FROM stdin;
\.


--
-- Name: bad_value_seq; Type: SEQUENCE SET; Schema: accounts; Owner: demoserver
--

SELECT pg_catalog.setval('bad_value_seq', 1, false);


--
-- Data for Name: bad_values; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY bad_values (bv_id, bv_name, bv_name_parts, bv_ip, bv_email) FROM stdin;
\.


--
-- Data for Name: badname_full; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY badname_full (bf_name) WITH OIDS FROM stdin;
\.


--
-- Data for Name: badname_partial; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY badname_partial (bp_name) WITH OIDS FROM stdin;
\.


--
-- Data for Name: gm_stats; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY gm_stats (gm_name, gm_acc_pos, gm_acc_neg, gm_name_pos, gm_name_neg, gm_race_pos, gm_race_neg) WITH OIDS FROM stdin;
\.


--
-- Data for Name: gm_stats_accs; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY gm_stats_accs (gm_name, gm_acc_id, accepted) WITH OIDS FROM stdin;
\.


--
-- Data for Name: gm_stats_chars; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY gm_stats_chars (gm_name, gm_char_name, accepted) WITH OIDS FROM stdin;
\.


--
-- Data for Name: gm_stats_race; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY gm_stats_race (gm_name, gm_apply_id, accepted) WITH OIDS FROM stdin;
\.


--
-- Data for Name: legtimulti; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY legtimulti (acc_id_1, acc_id_2, reason) WITH OIDS FROM stdin;
\.


--
-- Data for Name: pqxxlog_testserver; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY pqxxlog_testserver (name, date) WITH OIDS FROM stdin;
\.


--
-- Name: question_seq; Type: SEQUENCE SET; Schema: accounts; Owner: demoserver
--

SELECT pg_catalog.setval('question_seq', 1, false);


--
-- Data for Name: raceapplys; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY raceapplys (ra_accid, ra_race, ra_how, ra_why, ra_status, ra_answer) WITH OIDS FROM stdin;
\.


--
-- Data for Name: rights; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY rights (r_id, r_key_name, r_name_de, r_name_us, r_desc_de, r_desc_us) FROM stdin;
\.


--
-- Name: story_seq; Type: SEQUENCE SET; Schema: accounts; Owner: demoserver
--

SELECT pg_catalog.setval('story_seq', 1, false);


--
-- Data for Name: warnings; Type: TABLE DATA; Schema: accounts; Owner: demoserver
--

COPY warnings (wrn_accid, wrn_reason, wrn_gm, wrn_time) WITH OIDS FROM stdin;
\.


SET search_path = testserver, pg_catalog;

--
-- Data for Name: common; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY common (com_itemid, com_volume, com_weight, com_agingspeed, com_objectafterrot, com_script, com_rotsininventory, com_brightness, com_worth, com_buystack, com_maxstack) WITH OIDS FROM stdin;
56883633	339	5000	30000	255	339	\N	f	0	0	1	1
200889298	884	5000	30000	255	884	\N	f	0	0	1	1
200889300	886	5000	30000	255	886	\N	f	0	0	1	1
200889306	892	5000	30000	255	892	\N	f	0	0	1	1
200889307	893	5000	30000	255	893	\N	f	0	0	1	1
200889311	897	5000	30000	255	897	\N	f	0	0	1	1
200889312	898	5000	30000	255	898	\N	f	0	0	1	1
200889319	956	5000	30000	255	956	\N	f	0	0	1	1
200889320	957	5000	30000	255	957	\N	f	0	0	1	1
56883085	274	5000	30000	255	274	\N	f	0	0	1	1
56883491	2852	5000	30000	15	2854	\N	t	4	0	1	1
56883488	2851	5000	30000	15	2853	\N	t	4	0	1	1
56883645	2875	5000	30000	255	2875	\N	f	0	0	1	1
56883646	2880	5000	30000	255	2880	\N	f	0	0	1	1
56883825	2878	5000	30000	255	2878	\N	f	0	0	1	1
56883863	218	5000	30000	255	218	\N	f	0	0	1	1
56883864	2876	5000	30000	255	2876	\N	f	0	0	1	1
56883865	2877	5000	30000	255	2877	\N	f	0	0	1	1
56883084	273	5000	30000	255	273	\N	f	0	0	1	1
56883086	338	5000	30000	255	338	\N	f	0	0	1	1
56883141	301	5000	30000	255	301	\N	f	0	0	1	1
56883206	513	5000	30000	255	513	\N	f	0	0	1	1
56883173	1782	5000	30000	255	1782	\N	f	0	0	1	1
56883172	1783	5000	30000	255	1783	\N	f	0	0	1	1
56883383	1273	5000	30000	255	1273	\N	f	0	0	1	1
56883416	210	5000	30000	255	210	\N	f	0	0	1	1
56883303	587	5000	30000	8	586	\N	f	0	0	1	1
56883310	593	5000	30000	255	593	\N	f	0	0	1	1
56883311	594	5000	30000	255	594	\N	f	0	0	1	1
56883391	597	5000	30000	255	597	\N	f	0	0	1	1
56883393	598	5000	30000	255	598	\N	f	0	0	1	1
56883374	629	5000	30000	255	629	\N	f	0	0	1	1
56883479	1257	5000	30000	10	1273	\N	f	0	0	1	1
56883504	1276	5000	30000	255	1276	\N	f	0	0	1	1
56883500	1250	5000	30000	255	1250	\N	f	0	0	1	1
56883560	618	5000	30000	255	618	\N	f	0	0	1	1
56883561	619	5000	30000	255	619	\N	f	0	0	1	1
56883563	621	5000	30000	255	621	\N	f	0	0	1	1
56883564	622	5000	30000	255	622	\N	f	0	0	1	1
56883565	623	5000	30000	255	623	\N	f	0	0	1	1
56883568	626	5000	30000	255	626	\N	f	0	0	1	1
56883569	627	5000	30000	255	627	\N	f	0	0	1	1
56883570	628	5000	30000	255	628	\N	f	0	0	1	1
56883571	630	5000	30000	255	630	\N	f	0	0	1	1
56883476	1003	5000	30000	255	1003	\N	f	0	0	1	1
56883497	2488	5000	30000	255	2488	\N	f	0	0	1	1
56883502	1251	5000	30000	10	1250	\N	f	0	0	1	1
56883496	1246	5000	30000	255	1246	\N	f	0	0	1	1
56883747	203	5000	30000	255	203	\N	f	0	0	1	1
56883871	1812	5000	30000	255	1812	\N	f	0	0	1	1
56883870	1813	5000	30000	255	1813	\N	f	0	0	1	1
56883868	2169	5000	30000	255	2169	\N	f	0	0	1	1
56883869	2170	5000	30000	255	2170	\N	f	0	0	1	1
56884167	35	5000	30000	255	35	\N	f	0	0	1	1
56884168	2832	5000	30000	255	2832	\N	f	0	0	1	1
56883385	287	5000	30000	255	287	\N	f	0	0	1	1
12407541	877	5000	30000	255	877	\N	f	0	0	1	1
12407543	878	5000	30000	255	878	\N	f	0	0	1	1
56883562	620	5000	30000	255	620	\N	f	0	0	1	1
56883373	265	5000	30000	255	265	\N	f	0	0	1	1
56883187	396	5000	30000	15	395	\N	t	5	0	1	1
56883192	402	5000	30000	15	401	\N	t	5	0	1	1
63827956	910	255	255	255	910	\N	f	0	0	1	1
63827957	911	255	255	255	911	\N	f	0	0	1	1
63827958	912	255	255	255	912	\N	f	0	0	1	1
63827959	1810	255	255	255	1810	\N	f	0	0	1	1
56883106	387	5000	30000	255	387	\N	f	0	0	1	1
63827976	913	255	255	255	913	\N	f	0	0	1	1
56883108	12	5000	30000	4	314	\N	f	4	0	1	1
56883746	267	5000	30000	255	267	\N	f	0	0	1	1
34721091	3220	5000	30000	255	3220	\N	f	0	0	1	1
34721092	3221	5000	30000	255	3221	\N	f	0	0	1	1
34721093	3222	5000	30000	255	3222	\N	f	0	0	1	1
34721094	3223	5000	30000	255	3223	\N	f	0	0	1	1
34721115	3244	5000	30000	255	3244	\N	f	0	0	1	1
34721116	3245	5000	30000	255	3245	\N	f	0	0	1	1
34721117	3246	5000	30000	255	3246	\N	f	0	0	1	1
34721118	3247	5000	30000	255	3247	\N	f	0	0	1	1
34721119	3248	5000	30000	255	3248	\N	f	0	0	1	1
34721120	3249	5000	30000	255	3249	\N	f	0	0	1	1
34721121	3250	5000	30000	255	3250	\N	f	0	0	1	1
34721122	3251	5000	30000	255	3251	\N	f	0	0	1	1
34721123	3252	5000	30000	255	3252	\N	f	0	0	1	1
34721124	3253	5000	30000	255	3253	\N	f	0	0	1	1
34721125	3254	5000	30000	255	3254	\N	f	0	0	1	1
34721126	3255	5000	30000	255	3255	\N	f	0	0	1	1
34721127	3256	5000	30000	255	3256	\N	f	0	0	1	1
34721128	3257	5000	30000	255	3257	\N	f	0	0	1	1
34721129	3258	5000	30000	255	3258	\N	f	0	0	1	1
34721130	3259	5000	30000	255	3259	\N	f	0	0	1	1
34721133	3262	5000	30000	255	3262	\N	f	0	0	1	1
34721134	3263	5000	30000	255	3263	\N	f	0	0	1	1
34721135	3264	5000	30000	255	3264	\N	f	0	0	1	1
34721103	3232	5000	30000	255	3232	\N	f	0	0	1	1
34721107	3236	5000	30000	255	3236	\N	f	0	0	1	1
56883823	512	5000	30000	255	506	\N	f	0	0	1	1
58384575	692	5000	30000	255	692	\N	f	0	0	1	1
58384908	694	5000	30000	255	694	\N	f	0	0	1	1
56883867	2885	5000	30000	255	2885	\N	f	0	0	1	1
56883875	1807	5000	30000	255	1807	\N	f	0	0	1	1
56884068	1808	5000	30000	255	1808	\N	f	0	0	1	1
56884075	1817	5000	30000	255	1817	\N	f	0	0	1	1
56883489	2854	5000	30000	255	2854	\N	f	0	0	1	1
56883490	2853	5000	30000	255	2853	\N	f	0	0	1	1
56883139	586	5000	30000	255	586	\N	f	0	0	1	1
56883186	395	5000	30000	255	395	\N	f	0	0	1	1
56883191	401	5000	30000	255	401	\N	f	0	0	1	1
56884166	314	6	25	4	314	\N	f	0	100	1	1000
56883538	506	5000	30000	255	506	\N	f	0	0	1	1
\.


--
-- Data for Name: armor; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY armor (arm_itemid, arm_bodyparts, arm_puncture, arm_stroke, arm_thrust, arm_magicdisturbance, arm_absorb, arm_stiffness) WITH OIDS FROM stdin;
\.


--
-- Data for Name: attribute_packages; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY attribute_packages (attr_id, attr_name_de, attr_name_us, attr_str, attr_dex, attr_agi, attr_con, attr_per, attr_int, attr_wil, attr_ess) FROM stdin;
\.


--
-- Data for Name: char_log; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY char_log (cl_id, cl_acc_id, cl_char_id, cl_gm_id, cl_time, cl_message, cl_type) FROM stdin;
\.


--
-- Name: char_log_seq; Type: SEQUENCE SET; Schema: testserver; Owner: demoserver
--

SELECT pg_catalog.setval('char_log_seq', 25, false);


--
-- Data for Name: chars; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY chars (chr_accid, chr_playerid, chr_status, chr_statustime, chr_statusgm, chr_statusreason, chr_lastip, chr_onlinetime, chr_lastsavetime, chr_race, chr_sex, chr_prefix, chr_suffix, chr_name, chr_shortdesc_de, chr_shortdesc_us) WITH OIDS FROM stdin;
4456069	1	1	0	\N	\N	\N	0.0.0.0	0	0	0	0			Test		
\.


--
-- Data for Name: container; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY container (con_itemid, con_slots) WITH OIDS FROM stdin;
\.


--
-- Data for Name: deleted_chars; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY deleted_chars (dc_acc_id, dc_char_id, dc_char_name, dc_date) FROM stdin;
\.


--
-- Data for Name: gms; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY gms (gm_login, gm_charid, gm_rights_server, gm_rights_gmtool) WITH OIDS FROM stdin;
\.


--
-- Data for Name: player; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY player (ply_playerid, ply_posx, ply_posy, ply_posz, ply_faceto, ply_age, ply_weight, ply_body_height, ply_hitpoints, ply_mana, ply_attitude, ply_luck, ply_strength, ply_dexterity, ply_constitution, ply_agility, ply_intelligence, ply_perception, ply_willpower, ply_essence, ply_foodlevel, ply_appearance, ply_lifestate, ply_magictype, ply_magicflagsmage, ply_magicflagspriest, ply_magicflagsbard, ply_magicflagsdruid, ply_lastmusic, ply_poison, ply_mental_capacity, ply_dob, ply_hair, ply_beard, ply_hairred, ply_hairgreen, ply_hairblue, ply_skinred, ply_skingreen, ply_skinblue) WITH OIDS FROM stdin;
4456454	1	30	30	0	0	50	60	180	10000	0	0	0	10	10	10	10	10	10	10	10	30000	4	1	0	0	0	0	0	0	0	4000000	0	2	1	214	168	165	198	211	181
\.


--
-- Data for Name: gmpager; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY gmpager (pager_user, pager_text, pager_time, pager_status, pager_gm, pager_note, pager_id) WITH OIDS FROM stdin;
\.


--
-- Name: gmpager_seq; Type: SEQUENCE SET; Schema: testserver; Owner: demoserver
--

SELECT pg_catalog.setval('gmpager_seq', 1, false);


--
-- Data for Name: introduction; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY introduction (intro_player, intro_known_player) FROM stdin;
\.


--
-- Data for Name: itemname; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY itemname (itn_itemid, itn_german, itn_english) WITH OIDS FROM stdin;
56884173	597	Felswand	rock face
56884174	598	Felswand	rock face
56884337	513	Fässer	barrels
56884333	401	Fackelhalter	torch holder
56884334	402	Fackel	torch
56884778	1250	Fels	rock
56884779	1251	Fels	rock
56884781	1257	Fels	rock
56884785	1273	Fels	rock
56884508	35	Leiter	ladder
56884695	265	Gemälde	painting
56884697	267	Bücherregal	book shelf
56884703	273	Blume	flower
56884704	274	Farn	fern
56884717	287	Höhlenwand	cave wall
56884729	301	Hecke	hedge
56884786	1276	Fels	rock
56884900	1807	blühender Ceridern	blooming ceridern
56884901	1808	Ceridern-Baum	ceridern tree
56884887	1817	Scandrel-Kiefer	scandrel pine
56885039	2851	Kerzen	candles
56885040	2852	Kerzen	candles
56884776	1246	Fels	rock
56884902	1812	Farn	fern
56884903	1813	Farn	fern
56885041	2853	Kerzen	candles
56885042	2854	Kerzen	candles
56885008	2488	Kessel	kettle
56884947	2169	alter Baumstamm	old log
56884948	2170	alter Baumstamm	old log
56884889	1782	Gras	grass
56884890	1783	Gras	grass
56885035	2832	Höhleneingang	cave entry
56885143	586	Cachdern-Baum	cachdern tree
56885141	587	Baumstumpf	tree trunk
56885153	593	Baumstamm	fallen tree
56885154	594	Baumstamm	fallen tree
56885051	2875	Stuhl	chair
56885052	2876	Stuhl	chair
56885053	2877	Stuhl	chair
56885054	2878	Stuhl	chair
56885056	2880	Tisch	table
56885057	2885	Tisch	table
56884635	203	Palme	palm tree
56884217	630	Felswand	rock face
56885113	12	Lagerfeuer	camp fire
56885161	387	Strauch	bush
56884642	210	Hirschkopf	deer head
58384909	694	silberne Rüstung	silver armour
58384905	692	goldene Rüstung	golden armour
56884770	1003	Leiter	ladder
56884205	618	Steg	pier
56884206	619	Steg	pier
56884207	620	Steg	pier
56884208	621	Steg	pier
56884209	622	Felswand	rock face
56884210	623	Felswand	rock face
12407542	877	Steinmauer	stone wall
56884654	218	Sessel	armchair
12407544	878	Steinmauer	stone wall
56884346	338	Schilf	reed
56884347	339	Weinfass	wine barrel
56884739	314	Pottasche	potash
56884328	395	Öllampe	oil lamp
56884329	396	Öllampe	oil lamp
56884213	626	Felswand	rock face
56884214	627	Felswand	rock face
56884215	628	Felswand	rock face
56884216	629	Felswand	rock face
56884185	506	Wagen	cart
56884199	512	Fahne	flag
197675969	910	Baum	tree
197675970	911	Baum	tree
197675971	912	Baum	tree
197675972	913	Baum	tree
197676028	1810	Kaktus	cactus
197676133	3220	Mauer	wall
197676134	3221	Mauer	wall
197676135	3222	Mauer	wall
197676136	3223	Mauer	wall
197676145	3232	Tor	gate
197676149	3236	Tor	gate
197676157	3244	Mauer	wall
197676158	3245	Mauer	wall
197676159	3246	Mauer	wall
197676160	3247	Mauer	wall
197676161	3248	Fenster	window
197676162	3249	Fenster	window
197676163	3250	Fenster	window
197676164	3251	Fenster	window
197676165	3252	Zinnen	battlements
197676166	3253	Zinnen	battlements
197676167	3254	Zinnen	battlements
197676168	3255	Zinnen	battlements
197676169	3256	Zinnen	battlements
197676170	3257	Zinnen	battlements
197676171	3258	Zinnen	battlements
197676172	3259	Zinnen	battlements
197676175	3262	Kaktus	cactus
197676176	3263	Kaktus	cactus
197676177	3264	Kaktus	cactus
200905412	884	Steinkreis	circle of rocks
200905414	886	Steinkreis	circle of rocks
200905425	897	Pflanze	plant
200905426	898	Pflanze	plant
200905433	956	Pflanze	plant
200905434	957	Pflanze	plant
200905420	892	Palme	palm tree
200905421	893	Palme	palm tree
\.


--
-- Data for Name: longtimeeffects; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY longtimeeffects (lte_effectid, lte_effectname, lte_scriptname) WITH OIDS FROM stdin;
\.


--
-- Data for Name: mental_capacity; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY mental_capacity (mc_max_mentalcapacity, mc_learnborder, mc_first_warning, mc_second_warning, mc_decreaseamount, mc_increase_fighting, mc_increase_druidery, mc_increase_magic, mc_increase_craftmanship, mc_increase_other, mc_increase_language, mc_increasereference) WITH OIDS FROM stdin;
\.


--
-- Data for Name: monster; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY monster (mob_monsterid, mob_name_en, mob_race, mob_hitpoints, mob_movementtype, mob_canattack, mob_canhealself, script, mob_minsize, mob_maxsize, mob_name_de) WITH OIDS FROM stdin;
\.


--
-- Data for Name: monster_attributes; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY monster_attributes (mobattr_monsterid, mobattr_name, mobattr_min, mobattr_max) WITH OIDS FROM stdin;
\.


--
-- Data for Name: monster_items; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY monster_items (mobit_monsterid, mobit_position, mobit_itemid, mobit_mincount, mobit_maxcount) WITH OIDS FROM stdin;
\.


--
-- Data for Name: skillgroups; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY skillgroups (skg_group_id, skg_name_german, skg_name_english) FROM stdin;
\.


--
-- Data for Name: skills; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY skills (skl_skill_id, skl_group_id, skl_name, skl_name_german, skl_name_english) FROM stdin;
\.


--
-- Data for Name: monster_skills; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY monster_skills (mobsk_monsterid, mobsk_minvalue, mobsk_maxvalue, mobsk_skill_id) WITH OIDS FROM stdin;
\.


--
-- Data for Name: monsterattack; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY monsterattack (mat_race_type, mat_attack_type, mat_attack_value, mat_actionpointslost) WITH OIDS FROM stdin;
\.


--
-- Data for Name: naturalarmor; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY naturalarmor (nar_race, nar_strokearmor, nar_puncturearmor, nar_thrustarmor) WITH OIDS FROM stdin;
\.


--
-- Data for Name: npc; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY npc (npc_id, npc_type, npc_posx, npc_posy, npc_posz, npc_faceto, npc_is_healer, npc_name, npc_script, npc_sex, npc_hair, npc_beard, npc_hairred, npc_hairgreen, npc_hairblue, npc_skinred, npc_skingreen, npc_skinblue) WITH OIDS FROM stdin;
\.


--
-- Name: npc_seq; Type: SEQUENCE SET; Schema: testserver; Owner: demoserver
--

SELECT pg_catalog.setval('npc_seq', 1, false);


--
-- Data for Name: onlineplayer; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY onlineplayer (on_playerid) WITH OIDS FROM stdin;
\.


--
-- Data for Name: playeritems; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY playeritems (pit_itemid, pit_playerid, pit_linenumber, pit_in_container, pit_depot, pit_wear, pit_number, pit_quality, pit_data, pit_containerslot) WITH OIDS FROM stdin;
\.


--
-- Data for Name: playeritem_datavalues; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY playeritem_datavalues (idv_playerid, idv_linenumber, idv_key, idv_value) FROM stdin;
\.


--
-- Data for Name: playerlteffects; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY playerlteffects (plte_playerid, plte_effectid, plte_nextcalled, plte_numbercalled) WITH OIDS FROM stdin;
\.


--
-- Data for Name: playerlteffectvalues; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY playerlteffectvalues (pev_playerid, pev_effectid, pev_name, pev_value) WITH OIDS FROM stdin;
\.


--
-- Data for Name: playerskills; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY playerskills (psk_playerid, psk_skill_id, psk_value, psk_firsttry, psk_secondtry, psk_minor) WITH OIDS FROM stdin;
\.


--
-- Data for Name: questprogress; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY questprogress (qpg_userid, qpg_questid, qpg_progress) FROM stdin;
\.


--
-- Data for Name: quests; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY quests (qst_id, qst_script) FROM stdin;
\.


--
-- Data for Name: raceattr; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY raceattr (name, id, minage, maxage, minweight, maxweight, minbodyheight, maxbodyheight, minagility, maxagility, minconstitution, maxconstitution, mindexterity, maxdexterity, minessence, maxessence, minintelligence, maxintelligence, minperception, maxperception, minstrength, maxstrength, minwillpower, maxwillpower, posx, posy, posz, faceto, maxattribs, story_needed) WITH OIDS FROM stdin;
\.


--
-- Data for Name: scheduledscripts; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY scheduledscripts (sc_scriptname, sc_mincycletime, sc_maxcycletime, sc_functionname) WITH OIDS FROM stdin;
\.


--
-- Data for Name: scriptvariables; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY scriptvariables (svt_ids, svt_string) FROM stdin;
\.


--
-- Data for Name: spawnpoint; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY spawnpoint (spp_id, spp_x, spp_y, spp_z, spp_range, spp_minspawntime, spp_maxspawntime, spp_spawnall, spp_spawnrange) WITH OIDS FROM stdin;
\.


--
-- Data for Name: spawnpoint_monster; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY spawnpoint_monster (spm_id, spm_race, spm_count) WITH OIDS FROM stdin;
\.


--
-- Name: spawnpoint_seq; Type: SEQUENCE SET; Schema: testserver; Owner: demoserver
--

SELECT pg_catalog.setval('spawnpoint_seq', 1, false);


--
-- Data for Name: spells; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY spells (spl_spellid, spl_magictype, spl_scriptname) WITH OIDS FROM stdin;
\.


--
-- Data for Name: startpacks; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY startpacks (stp_id, stp_german, stp_english) FROM stdin;
\.


--
-- Data for Name: startpack_items; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY startpack_items (spi_id, spi_item_id, spi_linenumber, spi_number, spi_quality) FROM stdin;
\.


--
-- Data for Name: startpack_skills; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY startpack_skills (sps_id, sps_skill_id, sps_skill_value) FROM stdin;
\.


--
-- Data for Name: tiles; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY tiles (til_id, til_german, til_english, til_isnotpassable, til_isnottransparent, til_isnotpenetrateable, til_walkingcost, til_groundlevel, til_specialtile, til_script) WITH OIDS FROM stdin;
1583955	8	Dreck	dirt	0	0	0	40	0	0	\N
1583964	45	Teppich	carpet	0	0	0	25	0	0	\N
1583965	46	Teppich	carpet	0	0	0	25	0	0	\N
1583966	47	Teppich	carpet	0	0	0	25	0	0	\N
1583967	48	Teppich	carpet	0	0	0	25	0	0	\N
1583968	49	Teppich	carpet	0	0	0	25	0	0	\N
1583969	50	Teppich	carpet	0	0	0	25	0	0	\N
1583970	51	Teppich	carpet	0	0	0	25	0	0	\N
1583971	52	Teppich	carpet	0	0	0	25	0	0	\N
1583972	53	Teppich	carpet	0	0	0	25	0	0	\N
1583977	2	Fels	stones	0	0	0	50	0	0	\N
63826633	12	Sand	sand	0	0	0	30	0	0	\N
63826634	16	Dreck	mud	0	0	0	30	0	0	\N
1583957	3	Sand	Sand	0	0	0	30	0	0	\N
1583974	15	Höhlenboden	Dungeonfloor	0	0	0	20	0	0	\N
1583954	6	Wasser	water	1	0	0	20	0	1	\N
1583959	55	Marmor	marble	0	0	0	10	0	0	\N
209730370	60	Dach	roof	0	0	0	25	0	0	\N
63826635	17	Straße	street	0	0	0	10	0	0	\N
1583975	11	Wiese	grass	0	0	0	20	0	0	\N
\.


--
-- Data for Name: tilesmodificators; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY tilesmodificators (tim_itemid, tim_isnotpassable, tim_isnottransparent, tim_isnotpenetrateable, tim_specialitem, tim_groundlevel, tim_makepassable) WITH OIDS FROM stdin;
56891186	597	0	0	1	0	0	0
56891187	598	0	0	1	0	0	0
56891191	622	1	1	1	0	0	0
56891192	623	1	1	1	0	0	0
56891194	626	1	1	1	0	0	0
56891195	627	1	1	1	0	0	0
56891196	628	1	1	1	0	0	0
56891197	629	1	1	1	0	0	0
56891198	630	1	1	1	0	0	0
56891293	203	1	1	1	0	0	0
56891315	287	1	1	1	0	0	0
56891318	301	1	0	0	0	0	0
56891328	339	1	1	1	0	0	0
56891344	267	1	1	1	0	0	0
56891351	387	1	0	0	0	0	0
56891390	1251	1	0	0	0	0	0
56891392	1257	1	0	0	0	0	0
56891393	1250	1	0	0	0	0	0
56891394	1273	1	0	0	0	0	0
56891395	1276	1	0	0	0	0	0
56891398	1807	1	1	1	0	0	0
56891399	1808	1	1	1	0	0	0
56891401	1812	1	1	1	0	0	0
56891402	1813	1	1	1	0	0	0
56891403	1817	1	1	1	0	0	0
56891409	1246	1	0	0	0	0	0
56891443	506	1	0	1	0	0	0
56891449	512	1	0	1	0	0	0
56891450	513	1	0	0	0	0	0
56891479	12	0	0	0	1	0	0
56891480	35	0	0	0	1	0	0
56891481	2832	0	0	0	1	0	0
60419723	619	0	1	0	0	0	1
60419724	620	0	1	0	0	0	1
60419727	618	0	1	0	0	0	1
60419728	621	0	0	0	0	3	1
34721664	3220	1	1	1	0	0	0
34721665	3221	1	1	1	0	0	0
34721666	3222	1	1	1	0	0	0
34721667	3223	1	1	1	0	0	0
34721672	3232	1	1	1	0	0	0
34721674	3236	1	1	1	0	0	0
34721680	3244	1	1	1	0	0	0
34721681	3245	1	1	1	0	0	0
34721682	3246	1	1	1	0	0	0
34721683	3247	1	1	1	0	0	0
34721684	3252	1	1	1	0	0	0
34721685	3253	1	1	1	0	0	0
34721686	3254	1	1	1	0	0	0
34721687	3255	1	1	1	0	0	0
34721688	3256	1	1	1	0	0	0
34721689	3257	1	1	1	0	0	0
34721690	3258	1	1	1	0	0	0
34721691	3259	1	1	1	0	0	0
34721694	3262	1	1	1	0	0	0
34721695	3263	1	1	1	0	0	0
34721696	3264	1	1	1	0	0	0
34721761	3248	1	0	1	0	0	0
34721762	3249	1	0	1	0	0	0
34721763	3250	1	0	1	0	0	0
34721764	3251	1	0	1	0	0	0
163675277	586	1	0	1	0	0	0
168819075	910	1	0	1	0	0	0
168822459	911	1	1	1	0	0	0
168822460	912	1	1	1	0	0	0
168822461	913	1	1	1	0	0	0
168822475	692	1	1	1	0	0	0
168822476	694	1	1	1	0	0	0
168822708	1810	1	0	1	0	0	0
200889719	892	1	0	1	0	0	0
200889720	893	1	0	1	0	0	0
200889727	956	1	0	1	0	0	0
200889728	957	1	0	1	0	0	0
201915882	2488	1	1	1	0	0	0
202686230	877	1	1	1	0	0	0
202686231	878	1	1	1	0	0	0
\.


--
-- Data for Name: triggerfields; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY triggerfields (tgf_posx, tgf_posy, tgf_posz, tgf_script) WITH OIDS FROM stdin;
\.


--
-- Data for Name: weapon; Type: TABLE DATA; Schema: testserver; Owner: demoserver
--

COPY weapon (wp_itemid, wp_attack, wp_defence, wp_accuracy, wp_range, wp_weapontype, wp_ammunitiontype, wp_actionpoints, wp_magicdisturbance, wp_poison, wp_fightingscript) WITH OIDS FROM stdin;
\.


--
-- PostgreSQL database dump complete
--


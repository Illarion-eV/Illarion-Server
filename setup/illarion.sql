--
-- PostgreSQL database dump
--

SET statement_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;

--
-- Name: accounts; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA accounts;


--
-- Name: SCHEMA accounts; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON SCHEMA accounts IS 'accounts of illarion players';


--
-- Name: server; Type: SCHEMA; Schema: -; Owner: -
--

CREATE SCHEMA server;


--
-- Name: SCHEMA server; Type: COMMENT; Schema: -; Owner: -
--

COMMENT ON SCHEMA server IS 'development server';


SET search_path = accounts, pg_catalog;

--
-- Name: measuresystem; Type: TYPE; Schema: accounts; Owner: -
--

CREATE TYPE measuresystem AS ENUM (
    'metric',
    'imperial'
);


SET search_path = server, pg_catalog;

--
-- Name: cast_bool(boolean); Type: FUNCTION; Schema: server; Owner: -
--

CREATE FUNCTION cast_bool(boolean) RETURNS integer
    LANGUAGE plpgsql
    AS $_$
DECLARE
begin
if $1 then
return 1;
end if;
return 0;
end
$_$;


--
-- Name: generate_charid(); Type: FUNCTION; Schema: server; Owner: -
--

CREATE FUNCTION generate_charid() RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE
newid int4;
begin
loop
select (RANDOM()*256 * 256 * 256 * 128)::int4 from player into newid;
perform * from player where ply_playerid=newid;
if not found then
return newid;
end if;
end loop;
return 0;
end
$$;


--
-- Name: is_new_player(integer); Type: FUNCTION; Schema: server; Owner: -
--

CREATE FUNCTION is_new_player(account_id integer) RETURNS boolean
    LANGUAGE sql
    AS $_$select max(chr_onlinetime) < 3*60*60 as is_new_player from chars where chr_accid = $1;$_$;


--
-- Name: resort_items(integer); Type: FUNCTION; Schema: server; Owner: -
--

CREATE FUNCTION resort_items(integer) RETURNS boolean
    LANGUAGE plpgsql STRICT
    AS $_$DECLARE
        playerid ALIAS FOR $1;
        ammount int4;
        last_line int4;
        line_offset int4;
        curr_line int4;
        curr_item int4;
BEGIN
        SELECT COUNT(*) FROM playeritems WHERE pit_playerid = playerid INTO ammount;
        SELECT pit_linenumber FROM playeritems WHERE pit_playerid = playerid ORDER BY pit_linenumber DESC INTO last_line;
        SELECT 0::int4 INTO line_offset;
        IF ( ammount < last_line ) THEN
                LOOP
                        SELECT pit_linenumber, pit_itemid FROM playeritems WHERE pit_playerid = playerid ORDER BY pit_linenumber ASC LIMIT 1 OFFSET line_offset INTO curr_line, curr_item;
                        UPDATE playeritems SET pit_linenumber = ( line_offset + 1 ) WHERE pit_playerid = playerid AND pit_linenumber = curr_line;
                        PERFORM * FROM container WHERE con_itemid = curr_item;
                        IF FOUND THEN
                                UPDATE playeritems SET pit_in_container = ( line_offset + 1 ) WHERE pit_in_container = curr_line AND pit_playerid = playerid;
                        END IF;
                        SELECT ( line_offset + 1 )::int4 INTO line_offset;
                        IF ( line_offset = ammount ) THEN
                                RETURN TRUE;
                        END IF;
                END LOOP;
        ELSE
                RETURN FALSE;
        END IF;
        RETURN TRUE;
END$_$;


--
-- Name: somefunc(); Type: FUNCTION; Schema: server; Owner: -
--

CREATE FUNCTION somefunc() RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE
quantity integer := 30;
BEGIN
RAISE NOTICE 'Quantity here is %', quantity; -- Quantity here is 30
quantity := 50;
--
-- Create a subblock
--
DECLARE
quantity integer := 80;
BEGIN
RAISE NOTICE 'Quantity here is %', quantity; -- Quantity here is 80
END;

RAISE NOTICE 'Quantity here is %', quantity; -- Quantity here is 50

RETURN quantity;
END;
$$;


--
-- Name: test(); Type: FUNCTION; Schema: server; Owner: -
--

CREATE FUNCTION test() RETURNS integer
    LANGUAGE plpgsql
    AS $$
DECLARE
newid int4;
begin
loop
select (RANDOM()*256 * 256 * 256 * 128)::int4 from player into newid;
perform * from player where ply_playerid=newid;
if not found then
return newid;
end if;
end loop;
return 0;
end
$$;


SET search_path = accounts, pg_catalog;

--
-- Name: acc_group_seq; Type: SEQUENCE; Schema: accounts; Owner: -
--

CREATE SEQUENCE acc_group_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: acc_log_seq; Type: SEQUENCE; Schema: accounts; Owner: -
--

CREATE SEQUENCE acc_log_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


SET default_tablespace = '';

SET default_with_oids = true;

--
-- Name: account; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE account (
    acc_id integer DEFAULT nextval(('account_seq'::text)::regclass) NOT NULL,
    acc_login character varying(50) DEFAULT 'noname'::character varying NOT NULL,
    acc_passwd character varying(50) DEFAULT 'nopasswd'::character varying NOT NULL,
    acc_email character varying(50) DEFAULT 'noemail'::character varying NOT NULL,
    acc_registerdate timestamp without time zone,
    acc_lastip inet NOT NULL,
    acc_state integer DEFAULT 1 NOT NULL,
    acc_maxchars integer DEFAULT 5 NOT NULL,
    acc_lang smallint DEFAULT 0 NOT NULL,
    acc_newmail character varying(50),
    acc_racepermission character varying(255) DEFAULT '0,1,2,3,4,5'::character varying NOT NULL,
    acc_applypermission character varying DEFAULT '6,7,8'::character varying NOT NULL,
    acc_name character varying(50),
    acc_timeoffset integer DEFAULT 0 NOT NULL,
    acc_dst smallint DEFAULT 1 NOT NULL,
    acc_lastseen timestamp without time zone,
    acc_recv_inact_mails smallint DEFAULT 0 NOT NULL,
    acc_length measuresystem DEFAULT 'metric'::measuresystem NOT NULL,
    acc_weight measuresystem DEFAULT 'metric'::measuresystem NOT NULL
);


SET default_with_oids = false;

--
-- Name: account_groups; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE account_groups (
    ag_id integer DEFAULT nextval(('accounts.acc_group_seq'::text)::regclass) NOT NULL,
    ag_acc_id integer NOT NULL,
    ag_group_id integer NOT NULL
);


--
-- Name: account_log; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE account_log (
    al_id integer DEFAULT nextval(('acc_log_seq'::text)::regclass) NOT NULL,
    al_user_id integer NOT NULL,
    al_gm_id integer,
    al_time timestamp without time zone NOT NULL,
    al_message text NOT NULL,
    al_type smallint NOT NULL
);


--
-- Name: TABLE account_log; Type: COMMENT; Schema: accounts; Owner: -
--

COMMENT ON TABLE account_log IS 'Logfiles accountwork gmtool';


--
-- Name: account_seq; Type: SEQUENCE; Schema: accounts; Owner: -
--

CREATE SEQUENCE account_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: attribtemp; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE attribtemp (
    attr_id integer NOT NULL,
    attr_name_de character varying(50) NOT NULL,
    attr_name_us character varying(50) NOT NULL,
    attr_str smallint DEFAULT 2 NOT NULL,
    attr_dex smallint DEFAULT 2 NOT NULL,
    attr_agi smallint DEFAULT 2 NOT NULL,
    attr_con smallint DEFAULT 2 NOT NULL,
    attr_per smallint DEFAULT 2 NOT NULL,
    attr_int smallint DEFAULT 2 NOT NULL,
    attr_wil smallint DEFAULT 2 NOT NULL,
    attr_ess smallint DEFAULT 2 NOT NULL
);


--
-- Name: TABLE attribtemp; Type: COMMENT; Schema: accounts; Owner: -
--

COMMENT ON TABLE attribtemp IS 'Attribute Voreinstellung bei Charaktererschaffung';


SET default_with_oids = true;

--
-- Name: bad_ips; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE bad_ips (
    bip_ip character varying(15) NOT NULL
);


--
-- Name: TABLE bad_ips; Type: COMMENT; Schema: accounts; Owner: -
--

COMMENT ON TABLE bad_ips IS 'IPs that are not allowed to access the account system';


--
-- Name: COLUMN bad_ips.bip_ip; Type: COMMENT; Schema: accounts; Owner: -
--

COMMENT ON COLUMN bad_ips.bip_ip IS 'ip that is forbidden to access the account system';


--
-- Name: bad_value_seq; Type: SEQUENCE; Schema: accounts; Owner: -
--

CREATE SEQUENCE bad_value_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


SET default_with_oids = false;

--
-- Name: bad_values; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE bad_values (
    bv_id integer DEFAULT nextval(('bad_value_seq'::text)::regclass) NOT NULL,
    bv_name character varying(50),
    bv_name_parts character varying(50),
    bv_ip inet,
    bv_email character varying(50)
);


--
-- Name: TABLE bad_values; Type: COMMENT; Schema: accounts; Owner: -
--

COMMENT ON TABLE bad_values IS 'Invalid or banned values for account and chars';


SET default_with_oids = true;

--
-- Name: badname_full; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE badname_full (
    bf_name character varying(25) DEFAULT ''::character varying NOT NULL
);


--
-- Name: badname_partial; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE badname_partial (
    bp_name character varying(25) DEFAULT ''::character varying NOT NULL
);


--
-- Name: gm_stats; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE gm_stats (
    gm_name character varying(40) NOT NULL,
    gm_acc_pos integer DEFAULT 0 NOT NULL,
    gm_acc_neg integer DEFAULT 0 NOT NULL,
    gm_name_pos integer DEFAULT 0 NOT NULL,
    gm_name_neg integer DEFAULT 0 NOT NULL,
    gm_race_pos integer DEFAULT 0 NOT NULL,
    gm_race_neg integer DEFAULT 0 NOT NULL
);


--
-- Name: gm_stats_accs; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE gm_stats_accs (
    gm_name character varying(40) NOT NULL,
    gm_acc_id integer NOT NULL,
    accepted boolean NOT NULL
);


--
-- Name: gm_stats_chars; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE gm_stats_chars (
    gm_name character varying(40) NOT NULL,
    gm_char_name character varying(40) NOT NULL,
    accepted boolean NOT NULL
);


--
-- Name: gm_stats_race; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE gm_stats_race (
    gm_name character varying(40) NOT NULL,
    gm_apply_id integer DEFAULT 0 NOT NULL,
    accepted boolean DEFAULT true NOT NULL
);


SET default_with_oids = false;

--
-- Name: groups; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE groups (
    g_id integer NOT NULL,
    g_name_de character varying(50) NOT NULL,
    g_name_us character varying(50) NOT NULL,
    g_desc_de text NOT NULL,
    g_desc_us text NOT NULL,
    g_rights character varying(100) NOT NULL
);


SET default_with_oids = true;

--
-- Name: legtimulti; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE legtimulti (
    acc_id_1 integer NOT NULL,
    acc_id_2 integer NOT NULL,
    reason text
);


--
-- Name: question_seq; Type: SEQUENCE; Schema: accounts; Owner: -
--

CREATE SEQUENCE question_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: raceapplys; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE raceapplys (
    ra_accid integer NOT NULL,
    ra_race integer NOT NULL,
    ra_how text NOT NULL,
    ra_why text NOT NULL,
    ra_status smallint DEFAULT 0 NOT NULL,
    ra_answer text
);


SET default_with_oids = false;

--
-- Name: rights; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE rights (
    r_id integer NOT NULL,
    r_key_name character varying(40) NOT NULL,
    r_name_de character varying(50) NOT NULL,
    r_name_us character varying(50) NOT NULL,
    r_desc_de text NOT NULL,
    r_desc_us text NOT NULL
);


--
-- Name: story_seq; Type: SEQUENCE; Schema: accounts; Owner: -
--

CREATE SEQUENCE story_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: test; Type: VIEW; Schema: accounts; Owner: -
--

CREATE VIEW test AS
    SELECT raceapplys.ra_accid, raceapplys.ra_answer, gm_stats_race.gm_name FROM (raceapplys JOIN gm_stats_race ON ((raceapplys.oid = (gm_stats_race.gm_apply_id)::oid))) WHERE ((raceapplys.ra_race = 7) AND (raceapplys.ra_status = 2));


SET default_with_oids = true;

--
-- Name: warnings; Type: TABLE; Schema: accounts; Owner: -; Tablespace: 
--

CREATE TABLE warnings (
    wrn_accid integer NOT NULL,
    wrn_reason text NOT NULL,
    wrn_gm character varying(40) NOT NULL,
    wrn_time timestamp with time zone NOT NULL
);


SET search_path = server, pg_catalog;

--
-- Name: armor; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE armor (
    arm_itemid integer NOT NULL,
    arm_bodyparts smallint NOT NULL,
    arm_puncture smallint NOT NULL,
    arm_stroke smallint NOT NULL,
    arm_thrust smallint NOT NULL,
    arm_magicdisturbance integer NOT NULL,
    arm_absorb smallint DEFAULT 0 NOT NULL,
    arm_stiffness smallint DEFAULT 0 NOT NULL,
    arm_level smallint DEFAULT 0 NOT NULL,
    arm_type smallint DEFAULT 0 NOT NULL,
    CONSTRAINT armor_absorb_check CHECK ((arm_absorb >= 0)),
    CONSTRAINT armor_bodyparts_check CHECK (((arm_bodyparts >= 0) AND (arm_bodyparts < 256))),
    CONSTRAINT armor_level_check CHECK (((arm_level >= 0) AND (arm_level <= 100))),
    CONSTRAINT armor_magicdist_check CHECK (((arm_magicdisturbance >= 0) AND (arm_magicdisturbance <= 100))),
    CONSTRAINT armor_puncture_check CHECK (((arm_puncture >= 0) AND (arm_puncture <= 200))),
    CONSTRAINT armor_stiffness_check CHECK (((arm_stiffness >= 0) AND (arm_stiffness <= 200))),
    CONSTRAINT armor_stroke_check CHECK (((arm_stroke >= 0) AND (arm_stroke <= 200))),
    CONSTRAINT armor_thrust_check CHECK (((arm_thrust >= 0) AND (arm_thrust <= 200))),
    CONSTRAINT armor_type_check CHECK (((arm_type >= 0) AND (arm_type < 6)))
);


--
-- Name: itemname; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE itemname (
    itn_itemid integer DEFAULT 0 NOT NULL,
    itn_german character varying(30) DEFAULT 'unbekannt'::character varying NOT NULL,
    itn_english character varying(30) DEFAULT 'unknown'::character varying NOT NULL
);


--
-- Name: armor_info; Type: VIEW; Schema: server; Owner: -
--

CREATE VIEW armor_info AS
    SELECT armor.arm_itemid AS itemid, itemname.itn_english AS itemname, floor((((armor.arm_stroke * 100) / 150))::double precision) AS stroke_armor, floor((((armor.arm_thrust * 100) / 150))::double precision) AS thrust_armor, floor((((armor.arm_puncture * 100) / 150))::double precision) AS puncture_armor, floor((((armor.arm_stiffness * 100) / 150))::double precision) AS stiffness FROM (armor JOIN itemname ON ((armor.arm_itemid = itemname.itn_itemid)));


SET default_with_oids = false;

--
-- Name: attribute_packages; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE attribute_packages (
    attr_id integer NOT NULL,
    attr_name_de character varying(50) NOT NULL,
    attr_name_us character varying(50) NOT NULL,
    attr_str smallint DEFAULT 2 NOT NULL,
    attr_dex smallint DEFAULT 2 NOT NULL,
    attr_agi smallint DEFAULT 2 NOT NULL,
    attr_con smallint DEFAULT 2 NOT NULL,
    attr_per smallint DEFAULT 2 NOT NULL,
    attr_int smallint DEFAULT 2 NOT NULL,
    attr_wil smallint DEFAULT 2 NOT NULL,
    attr_ess smallint DEFAULT 2 NOT NULL
);


--
-- Name: TABLE attribute_packages; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON TABLE attribute_packages IS 'Attribute packages for character creation';


--
-- Name: char_log; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE char_log (
    cl_id integer DEFAULT nextval(('testserver.char_log_seq'::text)::regclass) NOT NULL,
    cl_acc_id integer NOT NULL,
    cl_char_id integer NOT NULL,
    cl_gm_id integer NOT NULL,
    cl_time timestamp without time zone NOT NULL,
    cl_message text NOT NULL,
    cl_type smallint NOT NULL
);


--
-- Name: char_log_seq; Type: SEQUENCE; Schema: server; Owner: -
--

CREATE SEQUENCE char_log_seq
    START WITH 25
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


SET default_with_oids = true;

--
-- Name: chars; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE chars (
    chr_accid integer NOT NULL,
    chr_playerid integer NOT NULL,
    chr_status integer NOT NULL,
    chr_statustime bigint,
    chr_statusgm integer,
    chr_statusreason character varying(250),
    chr_lastip character varying(15) DEFAULT '0.0.0.0'::character varying NOT NULL,
    chr_onlinetime bigint DEFAULT (0)::bigint NOT NULL,
    chr_lastsavetime bigint DEFAULT (0)::bigint NOT NULL,
    chr_race integer NOT NULL,
    chr_sex smallint NOT NULL,
    chr_name character varying(50) NOT NULL,
    chr_shortdesc_de character varying(255) DEFAULT ''::character varying NOT NULL,
    chr_shortdesc_us character varying(255) DEFAULT ''::character varying NOT NULL,
    CONSTRAINT character_sex CHECK (((chr_sex = (0)::smallint) OR (chr_sex = (1)::smallint)))
);


--
-- Name: common; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE common (
    com_itemid integer NOT NULL,
    com_volume integer NOT NULL,
    com_weight integer NOT NULL,
    com_agingspeed smallint DEFAULT (2)::smallint NOT NULL,
    com_objectafterrot integer NOT NULL,
    com_script character varying(50),
    com_rotsininventory boolean DEFAULT false NOT NULL,
    com_brightness smallint DEFAULT 0 NOT NULL,
    com_worth integer DEFAULT 0 NOT NULL,
    com_buystack integer DEFAULT 1 NOT NULL,
    com_maxstack smallint DEFAULT 1 NOT NULL,
    CONSTRAINT common_afterrot_check CHECK (((com_objectafterrot > 0) OR (com_itemid = 0))),
    CONSTRAINT common_aging_check CHECK ((com_agingspeed >= 0)),
    CONSTRAINT common_buystack_check CHECK ((com_buystack >= 1)),
    CONSTRAINT common_check CHECK ((((com_worth >= 20) OR (com_worth = 0)) OR ((com_itemid = 3076) AND (com_worth = 1)))),
    CONSTRAINT common_com_light_brightness_check CHECK (((com_brightness >= 0) AND (com_brightness <= 9))),
    CONSTRAINT common_com_maxstack_check CHECK ((com_maxstack <> 0)),
    CONSTRAINT common_script_check CHECK ((btrim((com_script)::text) <> ''::text)),
    CONSTRAINT common_volume_check CHECK ((com_volume >= 0)),
    CONSTRAINT common_weight_check CHECK ((com_weight >= 0))
);


--
-- Name: COLUMN common.com_itemid; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN common.com_itemid IS 'The unique ID of the item.';


--
-- Name: COLUMN common.com_script; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN common.com_script IS 'The name of the script module that handles his item.';


--
-- Name: COLUMN common.com_worth; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN common.com_worth IS 'The base price of the item that is used by the trader NPCs in the game.';


--
-- Name: COLUMN common.com_buystack; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN common.com_buystack IS 'Default stack size a vendor will offer of this item';


--
-- Name: container; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE container (
    con_itemid integer NOT NULL,
    con_slots integer NOT NULL,
    CONSTRAINT con_volume_check CHECK ((con_slots >= 0))
);


SET default_with_oids = false;

--
-- Name: deleted_chars; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE deleted_chars (
    dc_acc_id integer NOT NULL,
    dc_char_id integer NOT NULL,
    dc_char_name character varying(255) NOT NULL,
    dc_date timestamp without time zone DEFAULT now() NOT NULL
);


--
-- Name: gmpager_seq; Type: SEQUENCE; Schema: server; Owner: -
--

CREATE SEQUENCE gmpager_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


SET default_with_oids = true;

--
-- Name: gmpager; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE gmpager (
    pager_user integer NOT NULL,
    pager_text text NOT NULL,
    pager_time timestamp without time zone DEFAULT now() NOT NULL,
    pager_status smallint DEFAULT (0)::smallint NOT NULL,
    pager_gm integer,
    pager_note text,
    pager_id integer DEFAULT nextval('gmpager_seq'::regclass) NOT NULL
);


--
-- Name: COLUMN gmpager.pager_status; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN gmpager.pager_status IS '0, 1, 2 or 3';


--
-- Name: COLUMN gmpager.pager_gm; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN gmpager.pager_gm IS 'CharID';


--
-- Name: gms; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE gms (
    gm_login character varying(40) NOT NULL,
    gm_charid integer NOT NULL,
    gm_rights_server integer DEFAULT 0 NOT NULL,
    gm_rights_gmtool integer DEFAULT 0 NOT NULL
);


--
-- Name: gmrights; Type: VIEW; Schema: server; Owner: -
--

CREATE VIEW gmrights AS
    SELECT gms.gm_login, ((gms.gm_rights_server & 1) > 0) AS allow_login, ((gms.gm_rights_server & 2) > 0) AS basic_commands, ((gms.gm_rights_server & 4) > 0) AS warp, ((gms.gm_rights_server & 8) > 0) AS summon, ((gms.gm_rights_server & 16) > 0) AS prison, ((gms.gm_rights_server & 32) > 0) AS settiles, ((gms.gm_rights_server & 64) > 0) AS clipping, ((gms.gm_rights_server & 128) > 0) AS warp_field_edit, ((gms.gm_rights_server & 256) > 0) AS import, ((gms.gm_rights_server & 512) > 0) AS visibility, ((gms.gm_rights_server & 1024) > 0) AS table_reload, ((gms.gm_rights_server & 2048) > 0) AS ban, ((gms.gm_rights_server & 4096) > 0) AS toggle_login, ((gms.gm_rights_server & 8192) > 0) AS save, ((gms.gm_rights_server & 16384) > 0) AS broadcast, ((gms.gm_rights_server & 32768) > 0) AS forcelogout, ((gms.gm_rights_server & 65536) > 0) AS receive_gmcalls FROM gms;


SET default_with_oids = false;

--
-- Name: introduction; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE introduction (
    intro_player integer NOT NULL,
    intro_known_player integer NOT NULL
);


SET default_with_oids = true;

--
-- Name: longtimeeffects; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE longtimeeffects (
    lte_effectid smallint NOT NULL,
    lte_effectname character varying(50) NOT NULL,
    lte_scriptname character varying(50) NOT NULL,
    CONSTRAINT lte_script_check CHECK ((btrim((lte_scriptname)::text) <> ''::text))
);


--
-- Name: monster; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE monster (
    mob_monsterid integer NOT NULL,
    mob_name_en character varying(50) NOT NULL,
    mob_race smallint NOT NULL,
    mob_hitpoints smallint DEFAULT 10000 NOT NULL,
    mob_movementtype character varying(10) DEFAULT 'walk'::character varying NOT NULL,
    mob_canattack boolean DEFAULT true NOT NULL,
    mob_canhealself boolean DEFAULT true NOT NULL,
    script character varying(50),
    mob_minsize integer DEFAULT 0 NOT NULL,
    mob_maxsize integer DEFAULT 0 NOT NULL,
    mob_name_de character varying(50) NOT NULL,
    CONSTRAINT "$1" CHECK (((((mob_movementtype)::text = 'walk'::text) OR ((mob_movementtype)::text = 'fly'::text)) OR ((mob_movementtype)::text = 'crawl'::text))),
    CONSTRAINT monster_script_check CHECK ((btrim((script)::text) <> ''::text))
);


--
-- Name: monster_attributes; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE monster_attributes (
    mobattr_monsterid integer NOT NULL,
    mobattr_name character varying(20) NOT NULL,
    mobattr_min smallint NOT NULL,
    mobattr_max smallint NOT NULL,
    CONSTRAINT "$1" CHECK (((((((((((mobattr_name)::text = 'luck'::text) OR ((mobattr_name)::text = 'strength'::text)) OR ((mobattr_name)::text = 'dexterity'::text)) OR ((mobattr_name)::text = 'constitution'::text)) OR ((mobattr_name)::text = 'agility'::text)) OR ((mobattr_name)::text = 'intelligence'::text)) OR ((mobattr_name)::text = 'perception'::text)) OR ((mobattr_name)::text = 'willpower'::text)) OR ((mobattr_name)::text = 'essence'::text)))
);


--
-- Name: monster_items; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE monster_items (
    mobit_monsterid integer NOT NULL,
    mobit_position character varying(20) NOT NULL,
    mobit_itemid integer NOT NULL,
    mobit_mincount smallint NOT NULL,
    mobit_maxcount smallint NOT NULL,
    CONSTRAINT "$1" CHECK (((((((((((((((((((mobit_position)::text = 'head'::text) OR ((mobit_position)::text = 'neck'::text)) OR ((mobit_position)::text = 'breast'::text)) OR ((mobit_position)::text = 'hands'::text)) OR ((mobit_position)::text = 'left hand'::text)) OR ((mobit_position)::text = 'right hand'::text)) OR ((mobit_position)::text = 'left finger'::text)) OR ((mobit_position)::text = 'right finger'::text)) OR ((mobit_position)::text = 'legs'::text)) OR ((mobit_position)::text = 'feet'::text)) OR ((mobit_position)::text = 'coat'::text)) OR ((mobit_position)::text = 'belt1'::text)) OR ((mobit_position)::text = 'belt2'::text)) OR ((mobit_position)::text = 'belt3'::text)) OR ((mobit_position)::text = 'belt4'::text)) OR ((mobit_position)::text = 'belt5'::text)) OR ((mobit_position)::text = 'belt6'::text))),
    CONSTRAINT monster_items_count_check CHECK (((mobit_mincount <= mobit_maxcount) AND (mobit_mincount > 0))),
    CONSTRAINT monster_items_mobit_itemid_check CHECK ((mobit_itemid > 0))
);


--
-- Name: monster_skills; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE monster_skills (
    mobsk_monsterid integer DEFAULT 0 NOT NULL,
    mobsk_minvalue smallint DEFAULT (0)::smallint NOT NULL,
    mobsk_maxvalue smallint DEFAULT (0)::smallint NOT NULL,
    mobsk_skill_id integer NOT NULL,
    CONSTRAINT monster_skills_amount_check CHECK (((((mobsk_minvalue <= mobsk_maxvalue) AND (mobsk_minvalue >= 0)) AND (mobsk_maxvalue <= 100)) AND (mobsk_maxvalue > 0)))
);


--
-- Name: monsterattack; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE monsterattack (
    mat_race_type smallint NOT NULL,
    mat_attack_type smallint NOT NULL,
    mat_attack_value smallint NOT NULL,
    mat_actionpointslost smallint NOT NULL,
    CONSTRAINT monsterattack_actionspoints_check CHECK ((mat_actionpointslost >= 0)),
    CONSTRAINT monsterattack_attack_type_check CHECK (((mat_attack_type > 0) AND (mat_attack_type < 4))),
    CONSTRAINT monsterattack_attack_value_check CHECK ((mat_attack_value > 0))
);


--
-- Name: COLUMN monsterattack.mat_attack_type; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN monsterattack.mat_attack_type IS '1-slashing, 2-concussion, 3-puncture';


--
-- Name: naturalarmor; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE naturalarmor (
    nar_race smallint NOT NULL,
    nar_strokearmor smallint DEFAULT 0 NOT NULL,
    nar_puncturearmor smallint DEFAULT 0 NOT NULL,
    nar_thrustarmor smallint DEFAULT 0 NOT NULL,
    CONSTRAINT naturalarmor_armorvalues_check CHECK ((((nar_strokearmor >= 0) AND (nar_puncturearmor >= 0)) AND (nar_thrustarmor >= 0)))
);


--
-- Name: npc_seq; Type: SEQUENCE; Schema: server; Owner: -
--

CREATE SEQUENCE npc_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    MAXVALUE 2147483647
    CACHE 1;


--
-- Name: npc; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE npc (
    npc_id integer DEFAULT nextval('npc_seq'::regclass) NOT NULL,
    npc_type smallint NOT NULL,
    npc_posx smallint NOT NULL,
    npc_posy smallint NOT NULL,
    npc_posz smallint NOT NULL,
    npc_faceto smallint NOT NULL,
    npc_is_healer boolean DEFAULT false NOT NULL,
    npc_name character varying(50) NOT NULL,
    npc_script character varying(50) NOT NULL,
    npc_sex smallint DEFAULT 0 NOT NULL,
    npc_hair smallint DEFAULT 0 NOT NULL,
    npc_beard smallint DEFAULT 0 NOT NULL,
    npc_hairred smallint DEFAULT 255 NOT NULL,
    npc_hairgreen smallint DEFAULT 255 NOT NULL,
    npc_hairblue smallint DEFAULT 255 NOT NULL,
    npc_skinred smallint DEFAULT 255 NOT NULL,
    npc_skingreen smallint DEFAULT 255 NOT NULL,
    npc_skinblue smallint DEFAULT 255 NOT NULL,
    CONSTRAINT npc_beard_check CHECK ((npc_beard >= 0)),
    CONSTRAINT npc_faceto_check CHECK (((npc_faceto >= 0) AND (npc_faceto <= 7))),
    CONSTRAINT npc_hair_check CHECK ((npc_hair >= 0)),
    CONSTRAINT npc_hair_color_check CHECK ((((npc_hairred >= 0) AND (npc_hairblue >= 0)) AND (npc_hairgreen >= 0))),
    CONSTRAINT npc_script_check CHECK ((btrim((npc_script)::text) <> ''::text)),
    CONSTRAINT npc_sex_check CHECK (((npc_sex = 0) OR (npc_sex = 1))),
    CONSTRAINT npc_skincolor_check CHECK ((((npc_skinred >= 0) AND (npc_skinblue >= 0)) AND (npc_skingreen >= 0))),
    CONSTRAINT npc_type_check CHECK ((npc_type >= 0))
);


--
-- Name: onlineplayer; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE onlineplayer (
    on_playerid integer NOT NULL
);


--
-- Name: player; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE player (
    ply_playerid integer NOT NULL,
    ply_posx smallint DEFAULT 31 NOT NULL,
    ply_posy smallint DEFAULT 22 NOT NULL,
    ply_posz smallint DEFAULT 100 NOT NULL,
    ply_faceto smallint DEFAULT 0 NOT NULL,
    ply_age smallint DEFAULT 0 NOT NULL,
    ply_weight integer DEFAULT 0 NOT NULL,
    ply_body_height smallint DEFAULT (0)::smallint NOT NULL,
    ply_hitpoints smallint DEFAULT 10000 NOT NULL,
    ply_mana smallint DEFAULT 0 NOT NULL,
    ply_attitude smallint DEFAULT (0)::smallint NOT NULL,
    ply_luck smallint DEFAULT (0)::smallint NOT NULL,
    ply_strength smallint DEFAULT (0)::smallint NOT NULL,
    ply_dexterity smallint DEFAULT (0)::smallint NOT NULL,
    ply_constitution smallint DEFAULT (0)::smallint NOT NULL,
    ply_agility smallint DEFAULT (0)::smallint NOT NULL,
    ply_intelligence smallint DEFAULT (0)::smallint NOT NULL,
    ply_perception smallint DEFAULT (0)::smallint NOT NULL,
    ply_willpower smallint DEFAULT (0)::smallint NOT NULL,
    ply_essence smallint DEFAULT (0)::smallint NOT NULL,
    ply_foodlevel integer DEFAULT 0 NOT NULL,
    ply_appearance integer DEFAULT 0 NOT NULL,
    ply_lifestate smallint DEFAULT (1)::smallint NOT NULL,
    ply_magictype smallint DEFAULT (0)::smallint NOT NULL,
    ply_magicflagsmage bigint DEFAULT 0 NOT NULL,
    ply_magicflagspriest bigint DEFAULT 0 NOT NULL,
    ply_magicflagsbard bigint DEFAULT 0 NOT NULL,
    ply_magicflagsdruid bigint DEFAULT 0 NOT NULL,
    ply_lastmusic integer DEFAULT 0 NOT NULL,
    ply_poison smallint DEFAULT 0 NOT NULL,
    ply_mental_capacity integer DEFAULT 2000000 NOT NULL,
    ply_dob integer DEFAULT 0 NOT NULL,
    ply_hair smallint DEFAULT 0 NOT NULL,
    ply_beard smallint DEFAULT 0 NOT NULL,
    ply_hairred smallint DEFAULT 255 NOT NULL,
    ply_hairgreen smallint DEFAULT 255 NOT NULL,
    ply_hairblue smallint DEFAULT 255 NOT NULL,
    ply_skinred smallint DEFAULT 255 NOT NULL,
    ply_skingreen smallint DEFAULT 255 NOT NULL,
    ply_skinblue smallint DEFAULT 255 NOT NULL,
    CONSTRAINT player_age_check CHECK ((ply_age >= 18)),
    CONSTRAINT player_agility_check CHECK (((ply_agility >= 0) AND (ply_agility <= 250))),
    CONSTRAINT player_appearance_check CHECK ((ply_appearance >= 0)),
    CONSTRAINT player_beard_check CHECK ((ply_beard >= 0)),
    CONSTRAINT player_constitution_check CHECK (((ply_constitution >= 0) AND (ply_constitution <= 250))),
    CONSTRAINT player_dexterity_check CHECK (((ply_dexterity >= 0) AND (ply_dexterity <= 250))),
    CONSTRAINT player_dob_check CHECK ((ply_dob >= 0)),
    CONSTRAINT player_essence_check CHECK (((ply_essence >= 0) AND (ply_essence <= 250))),
    CONSTRAINT player_faceto_check CHECK (((ply_faceto >= 0) AND (ply_faceto <= 7))),
    CONSTRAINT player_foodlevel_check CHECK (((ply_foodlevel >= 0) AND (ply_foodlevel <= 60000))),
    CONSTRAINT player_hair_check CHECK ((ply_hair >= 0)),
    CONSTRAINT player_haircolor_check CHECK ((((ply_hairred >= 0) AND (ply_hairgreen >= 0)) AND (ply_hairblue >= 0))),
    CONSTRAINT player_height_check CHECK ((ply_body_height >= 0)),
    CONSTRAINT player_hitpoints_check CHECK (((ply_hitpoints >= 0) AND (ply_hitpoints <= 10000))),
    CONSTRAINT player_intelligence_check CHECK (((ply_intelligence >= 0) AND (ply_intelligence <= 250))),
    CONSTRAINT player_lastmusic_check CHECK ((ply_lastmusic >= 0)),
    CONSTRAINT player_lifestate_check CHECK (((ply_lifestate = 0) OR (ply_lifestate = 1))),
    CONSTRAINT player_magicflagsbard_check CHECK (((ply_magicflagsbard >= 0) AND (ply_magicflagsbard <= 8589934591::bigint))),
    CONSTRAINT player_magicflagsdruid_check CHECK (((ply_magicflagsdruid >= 0) AND (ply_magicflagsdruid <= 8589934591::bigint))),
    CONSTRAINT player_magicflagsmage_check CHECK (((ply_magicflagsmage >= 0) AND (ply_magicflagsmage <= 8589934591::bigint))),
    CONSTRAINT player_magicflagspriest_check CHECK (((ply_magicflagspriest >= 0) AND (ply_magicflagspriest <= 8589934591::bigint))),
    CONSTRAINT player_magictype_check CHECK (((ply_magictype >= 0) AND (ply_magictype <= 3))),
    CONSTRAINT player_mana_check CHECK (((ply_mana >= 0) AND (ply_mana <= 10000))),
    CONSTRAINT player_mental_capacity_check CHECK ((ply_mental_capacity >= 0)),
    CONSTRAINT player_perception_check CHECK (((ply_perception >= 0) AND (ply_perception <= 250))),
    CONSTRAINT player_poison_check CHECK (((ply_poison >= 0) AND (ply_poison <= 10000))),
    CONSTRAINT player_skincolor_check CHECK ((((ply_skinred >= 0) AND (ply_skingreen >= 0)) AND (ply_skinblue >= 0))),
    CONSTRAINT player_strength_check CHECK (((ply_strength >= 0) AND (ply_strength <= 250))),
    CONSTRAINT player_weight_check CHECK ((ply_weight >= 0)),
    CONSTRAINT player_willpower_check CHECK (((ply_willpower >= 0) AND (ply_willpower <= 250)))
);


SET default_with_oids = false;

--
-- Name: playeritem_datavalues; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE playeritem_datavalues (
    idv_playerid integer NOT NULL,
    idv_linenumber smallint NOT NULL,
    idv_key character varying(255) NOT NULL,
    idv_value character varying(255) DEFAULT ''::character varying NOT NULL
);


SET default_with_oids = true;

--
-- Name: playeritems; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE playeritems (
    pit_itemid integer DEFAULT 0 NOT NULL,
    pit_playerid integer NOT NULL,
    pit_linenumber smallint NOT NULL,
    pit_in_container smallint DEFAULT 0 NOT NULL,
    pit_depot smallint DEFAULT 0 NOT NULL,
    pit_wear smallint DEFAULT (0)::smallint NOT NULL,
    pit_number smallint DEFAULT (0)::smallint NOT NULL,
    pit_quality smallint DEFAULT 330 NOT NULL,
    pit_containerslot smallint DEFAULT 0 NOT NULL,
    CONSTRAINT playeritems_depot_check CHECK ((pit_depot >= 0)),
    CONSTRAINT playeritems_in_container_check CHECK (((pit_in_container < pit_linenumber) OR (pit_in_container = 0))),
    CONSTRAINT playeritems_linenumber_check CHECK ((pit_linenumber > 0)),
    CONSTRAINT playeritems_pit_itemid_check CHECK ((pit_itemid > 0)),
    CONSTRAINT playeritems_wear_check CHECK (((pit_wear >= 0) AND (pit_wear <= 255)))
);


--
-- Name: playerlteffects; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE playerlteffects (
    plte_playerid integer NOT NULL,
    plte_effectid smallint DEFAULT 0 NOT NULL,
    plte_nextcalled integer DEFAULT 0 NOT NULL,
    plte_numbercalled integer DEFAULT 0 NOT NULL
);


--
-- Name: playerlteffectvalues; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE playerlteffectvalues (
    pev_playerid integer NOT NULL,
    pev_effectid smallint NOT NULL,
    pev_name character varying(30) DEFAULT 'default'::character varying NOT NULL,
    pev_value bigint DEFAULT 0 NOT NULL
);


--
-- Name: playerskills; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE playerskills (
    psk_playerid integer DEFAULT 0 NOT NULL,
    psk_skill_id integer NOT NULL,
    psk_value smallint DEFAULT 0 NOT NULL,
    psk_firsttry integer DEFAULT 0 NOT NULL,
    psk_secondtry integer DEFAULT 0 NOT NULL,
    psk_minor integer DEFAULT 0 NOT NULL,
    CONSTRAINT playerskills_value_check CHECK (((psk_value >= 0) AND (psk_value <= 100)))
);


SET default_with_oids = false;

--
-- Name: questprogress; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE questprogress (
    qpg_userid integer NOT NULL,
    qpg_questid smallint NOT NULL,
    qpg_progress integer DEFAULT 0 NOT NULL,
    qpg_time integer DEFAULT 0 NOT NULL
);


--
-- Name: quests; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE quests (
    qst_id smallint NOT NULL,
    qst_script character varying(50) NOT NULL
);


SET default_with_oids = true;

--
-- Name: raceattr; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE raceattr (
    name character varying(30) NOT NULL,
    id integer NOT NULL,
    minage integer DEFAULT 18,
    maxage integer DEFAULT 80,
    minweight integer DEFAULT 4000,
    maxweight integer DEFAULT 12000,
    minbodyheight integer DEFAULT 55,
    maxbodyheight integer DEFAULT 85,
    minagility integer DEFAULT 2,
    maxagility integer DEFAULT 20,
    minconstitution integer DEFAULT 2,
    maxconstitution integer DEFAULT 20,
    mindexterity integer DEFAULT 2,
    maxdexterity integer DEFAULT 20,
    minessence integer DEFAULT 2,
    maxessence integer DEFAULT 20,
    minintelligence integer DEFAULT 2,
    maxintelligence integer DEFAULT 20,
    minperception integer DEFAULT 2,
    maxperception integer DEFAULT 20,
    minstrength integer DEFAULT 2,
    maxstrength integer DEFAULT 20,
    minwillpower integer DEFAULT 2,
    maxwillpower integer DEFAULT 20,
    posx integer DEFAULT 0,
    posy integer DEFAULT 0,
    posz integer DEFAULT 0,
    faceto integer DEFAULT 0,
    maxattribs smallint DEFAULT 84,
    story_needed boolean DEFAULT false
);


--
-- Name: scheduledscripts; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE scheduledscripts (
    sc_scriptname character varying(50) NOT NULL,
    sc_mincycletime integer NOT NULL,
    sc_maxcycletime integer NOT NULL,
    sc_functionname character varying(50) NOT NULL,
    CONSTRAINT schedule_function_check CHECK ((btrim((sc_functionname)::text) <> ''::text)),
    CONSTRAINT schedule_script_name CHECK ((btrim((sc_scriptname)::text) <> ''::text)),
    CONSTRAINT scs_cycletime_check CHECK (((sc_mincycletime > 0) AND (sc_mincycletime <= sc_maxcycletime)))
);


SET default_with_oids = false;

--
-- Name: scriptvariables; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE scriptvariables (
    svt_ids character varying(255) NOT NULL,
    svt_string character varying(255) NOT NULL
);


--
-- Name: skillgroups; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE skillgroups (
    skg_group_id integer NOT NULL,
    skg_name_german character varying(45) NOT NULL,
    skg_name_english character varying(45),
    CONSTRAINT skg_name_check CHECK ((btrim((skg_name_german)::text) <> ''::text))
);


--
-- Name: TABLE skillgroups; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON TABLE skillgroups IS 'This table contains the groups a skill can be assigned to.';


--
-- Name: skills; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE skills (
    skl_skill_id integer NOT NULL,
    skl_group_id integer NOT NULL,
    skl_name character varying(20) NOT NULL,
    skl_name_german character varying(45) NOT NULL,
    skl_name_english character varying(45) NOT NULL,
    CONSTRAINT skl_name_check CHECK ((btrim((skl_name)::text) <> ''::text)),
    CONSTRAINT skl_name_english_check CHECK ((btrim((skl_name_english)::text) <> ''::text)),
    CONSTRAINT skl_name_german_check CHECK ((btrim((skl_name_german)::text) <> ''::text))
);


--
-- Name: TABLE skills; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON TABLE skills IS 'This table contains the existing skills';


--
-- Name: spawnpoint_seq; Type: SEQUENCE; Schema: server; Owner: -
--

CREATE SEQUENCE spawnpoint_seq
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    MAXVALUE 2147483647
    CACHE 1;


SET default_with_oids = true;

--
-- Name: spawnpoint; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE spawnpoint (
    spp_id integer DEFAULT (nextval('spawnpoint_seq'::regclass))::integer NOT NULL,
    spp_x smallint NOT NULL,
    spp_y smallint NOT NULL,
    spp_z smallint NOT NULL,
    spp_range smallint DEFAULT 20 NOT NULL,
    spp_minspawntime smallint DEFAULT 2 NOT NULL,
    spp_maxspawntime smallint DEFAULT 12 NOT NULL,
    spp_spawnall boolean DEFAULT false NOT NULL,
    spp_spawnrange smallint DEFAULT 5 NOT NULL,
    CONSTRAINT spawnpoint_range_check CHECK (((spp_spawnrange > 0) AND (spp_range >= spp_spawnrange))),
    CONSTRAINT spawnpoint_time_check CHECK (((spp_minspawntime > 0) AND (spp_maxspawntime >= spp_minspawntime)))
);


--
-- Name: spawnpoint_monster; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE spawnpoint_monster (
    spm_id integer DEFAULT (currval('spawnpoint_seq'::regclass))::integer NOT NULL,
    spm_race integer NOT NULL,
    spm_count smallint NOT NULL,
    CONSTRAINT spawnpointmonsters_count_check CHECK ((spm_count > 0))
);


--
-- Name: spells; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE spells (
    spl_spellid integer NOT NULL,
    spl_magictype smallint NOT NULL,
    spl_scriptname character varying(50) NOT NULL,
    CONSTRAINT spell_script_check CHECK ((btrim((spl_scriptname)::text) <> ''::text)),
    CONSTRAINT spell_type_check CHECK (((spl_magictype >= 0) AND (spl_magictype <= 3)))
);


SET default_with_oids = false;

--
-- Name: startpack_items; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE startpack_items (
    spi_id integer NOT NULL,
    spi_item_id integer NOT NULL,
    spi_linenumber smallint NOT NULL,
    spi_number smallint NOT NULL,
    spi_quality smallint NOT NULL,
    CONSTRAINT startpack_items_linenumber_check CHECK (((spi_linenumber > 0) AND (spi_linenumber <= 18))),
    CONSTRAINT startpack_items_number_check CHECK ((spi_number > 0)),
    CONSTRAINT startpack_items_quality_check CHECK (((spi_quality > 0) AND (spi_quality < 1000))),
    CONSTRAINT startpack_items_spi_item_id_check CHECK ((spi_item_id > 0))
);


--
-- Name: TABLE startpack_items; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON TABLE startpack_items IS 'This table contains the items that belong to each starting package.';


--
-- Name: COLUMN startpack_items.spi_id; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_items.spi_id IS 'The ID of the package';


--
-- Name: COLUMN startpack_items.spi_item_id; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_items.spi_item_id IS 'The ID of the item';


--
-- Name: COLUMN startpack_items.spi_linenumber; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_items.spi_linenumber IS 'The line number this item will get assigned to';


--
-- Name: COLUMN startpack_items.spi_number; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_items.spi_number IS 'The amount of items of this type';


--
-- Name: COLUMN startpack_items.spi_quality; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_items.spi_quality IS 'The quality of the item';


--
-- Name: startpack_skills; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE startpack_skills (
    sps_id integer NOT NULL,
    sps_skill_id integer NOT NULL,
    sps_skill_value smallint NOT NULL,
    CONSTRAINT startpack_skills_value_check CHECK (((sps_skill_value > 0) AND (sps_skill_value <= 100)))
);


--
-- Name: TABLE startpack_skills; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON TABLE startpack_skills IS 'This table contains the skills each starting package contains.';


--
-- Name: COLUMN startpack_skills.sps_id; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_skills.sps_id IS 'The ID of the package';


--
-- Name: COLUMN startpack_skills.sps_skill_id; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_skills.sps_skill_id IS 'The ID of the skill';


--
-- Name: COLUMN startpack_skills.sps_skill_value; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpack_skills.sps_skill_value IS 'The value of the skill';


--
-- Name: startpacks; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE startpacks (
    stp_id integer NOT NULL,
    stp_german character varying(20) NOT NULL,
    stp_english character varying(20) NOT NULL,
    CONSTRAINT startpacks_english_lenght CHECK ((btrim((stp_english)::text) <> ''::text)),
    CONSTRAINT startpacks_german_lenght CHECK ((btrim((stp_german)::text) <> ''::text))
);


--
-- Name: TABLE startpacks; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON TABLE startpacks IS 'This table contains the starting packages the players can choose from when creating a new character.';


--
-- Name: COLUMN startpacks.stp_id; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpacks.stp_id IS 'The ID of the package';


--
-- Name: COLUMN startpacks.stp_german; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpacks.stp_german IS 'The German name of the package';


--
-- Name: COLUMN startpacks.stp_english; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN startpacks.stp_english IS 'The English name of the package';


SET default_with_oids = true;

--
-- Name: tiles; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE tiles (
    til_id integer DEFAULT 0 NOT NULL,
    til_german character varying(30) DEFAULT 'unbekannt'::character varying NOT NULL,
    til_english character varying(30) DEFAULT 'unknown'::character varying NOT NULL,
    til_isnotpassable smallint DEFAULT (0)::smallint NOT NULL,
    til_isnottransparent smallint DEFAULT (0)::smallint NOT NULL,
    til_isnotpenetrateable smallint DEFAULT (0)::smallint NOT NULL,
    til_walkingcost smallint DEFAULT (0)::smallint NOT NULL,
    til_groundlevel smallint DEFAULT (0)::smallint NOT NULL,
    til_specialtile smallint DEFAULT (0)::smallint NOT NULL,
    til_script character varying(50),
    CONSTRAINT tiles_notpassable_check CHECK (((til_isnotpassable = (0)::smallint) OR (til_isnotpassable = (1)::smallint))),
    CONSTRAINT tiles_notpenetrateable_check CHECK (((til_isnotpenetrateable = (0)::smallint) OR (til_isnotpenetrateable = (1)::smallint))),
    CONSTRAINT tiles_nottransparent_check CHECK (((til_isnottransparent = (0)::smallint) OR (til_isnottransparent = (1)::smallint))),
    CONSTRAINT tiles_script_check CHECK ((btrim((til_script)::text) <> ''::text)),
    CONSTRAINT tiles_specialtile_check CHECK (((til_specialtile = (0)::smallint) OR (til_specialtile = (1)::smallint))),
    CONSTRAINT tiles_walkingcost_check CHECK ((til_walkingcost >= 0))
);


--
-- Name: tilesmodificators; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE tilesmodificators (
    tim_itemid integer DEFAULT 0 NOT NULL,
    tim_isnotpassable smallint DEFAULT 0 NOT NULL,
    tim_isnottransparent smallint DEFAULT 0 NOT NULL,
    tim_isnotpenetrateable smallint DEFAULT 0 NOT NULL,
    tim_specialitem smallint DEFAULT 0 NOT NULL,
    tim_groundlevel smallint DEFAULT 0 NOT NULL,
    tim_makepassable smallint DEFAULT 0 NOT NULL,
    CONSTRAINT tilesmod_makepassable_check CHECK (((tim_makepassable = (0)::smallint) OR (tim_makepassable = (1)::smallint))),
    CONSTRAINT tilesmod_notpassable_check CHECK (((tim_isnotpassable = (0)::smallint) OR (tim_isnotpassable = (1)::smallint))),
    CONSTRAINT tilesmod_notpenetrateable_check CHECK (((tim_isnotpenetrateable = (0)::smallint) OR (tim_isnotpenetrateable = (1)::smallint))),
    CONSTRAINT tilesmod_nottransparent_check CHECK (((tim_isnottransparent = (0)::smallint) OR (tim_isnottransparent = (1)::smallint))),
    CONSTRAINT tilesmod_specialitem_check CHECK (((tim_specialitem = (0)::smallint) OR (tim_specialitem = (1)::smallint)))
);


--
-- Name: triggerfields; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE triggerfields (
    tgf_posx smallint NOT NULL,
    tgf_posy smallint NOT NULL,
    tgf_posz smallint NOT NULL,
    tgf_script character varying(50) NOT NULL,
    CONSTRAINT trigger_script_check CHECK ((btrim((tgf_script)::text) <> ''::text))
);


--
-- Name: weapon; Type: TABLE; Schema: server; Owner: -; Tablespace: 
--

CREATE TABLE weapon (
    wp_itemid integer NOT NULL,
    wp_attack smallint NOT NULL,
    wp_defence smallint NOT NULL,
    wp_accuracy smallint NOT NULL,
    wp_range smallint NOT NULL,
    wp_weapontype smallint NOT NULL,
    wp_ammunitiontype smallint NOT NULL,
    wp_actionpoints smallint NOT NULL,
    wp_magicdisturbance integer NOT NULL,
    wp_poison smallint NOT NULL,
    wp_fightingscript character varying(50),
    wp_level smallint DEFAULT 0 NOT NULL,
    CONSTRAINT weapon_accuracy_check CHECK (((wp_accuracy > 0) AND (wp_accuracy <= 100))),
    CONSTRAINT weapon_ap_check CHECK (((wp_actionpoints > 0) AND (wp_actionpoints <= 100))),
    CONSTRAINT weapon_attack_check CHECK (((wp_attack >= 0) AND (wp_attack <= 200))),
    CONSTRAINT weapon_defence_check CHECK (((wp_defence >= 0) AND (wp_defence <= 200))),
    CONSTRAINT weapon_level_check CHECK (((wp_level >= 0) AND (wp_level <= 100))),
    CONSTRAINT weapon_magicdist_check CHECK (((wp_magicdisturbance >= 0) AND (wp_magicdisturbance <= 200))),
    CONSTRAINT weapon_poison_check CHECK (((wp_poison >= 0) AND (wp_poison <= 100))),
    CONSTRAINT weapon_range_check CHECK (((wp_range > 0) AND (wp_range <= 10))),
    CONSTRAINT weapon_script_check CHECK (((wp_fightingscript)::text <> ''::text)),
    CONSTRAINT weapon_type_check CHECK ((wp_weapontype > 0))
);


--
-- Name: COLUMN weapon.wp_fightingscript; Type: COMMENT; Schema: server; Owner: -
--

COMMENT ON COLUMN weapon.wp_fightingscript IS 'script name';


SET search_path = accounts, pg_catalog;

--
-- Name: acc_name_unique; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY account
    ADD CONSTRAINT acc_name_unique UNIQUE (acc_login);


--
-- Name: account_groups_pkey; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY account_groups
    ADD CONSTRAINT account_groups_pkey PRIMARY KEY (ag_id);


--
-- Name: account_log_al_id_key; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY account_log
    ADD CONSTRAINT account_log_al_id_key UNIQUE (al_id);


--
-- Name: account_pkey; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY account
    ADD CONSTRAINT account_pkey PRIMARY KEY (acc_id);


--
-- Name: attribtemp_attr_id_key; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY attribtemp
    ADD CONSTRAINT attribtemp_attr_id_key UNIQUE (attr_id);


--
-- Name: bad_values_bv_id_key; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY bad_values
    ADD CONSTRAINT bad_values_bv_id_key UNIQUE (bv_id);


--
-- Name: badname_full_pkey; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY badname_full
    ADD CONSTRAINT badname_full_pkey PRIMARY KEY (bf_name);


--
-- Name: badname_partial_pkey; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY badname_partial
    ADD CONSTRAINT badname_partial_pkey PRIMARY KEY (bp_name);


--
-- Name: groups_pkey; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY groups
    ADD CONSTRAINT groups_pkey PRIMARY KEY (g_id);


--
-- Name: legtimulti_pkey; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY legtimulti
    ADD CONSTRAINT legtimulti_pkey PRIMARY KEY (acc_id_1, acc_id_2);


--
-- Name: rights_pkey; Type: CONSTRAINT; Schema: accounts; Owner: -; Tablespace: 
--

ALTER TABLE ONLY rights
    ADD CONSTRAINT rights_pkey PRIMARY KEY (r_id);


SET search_path = server, pg_catalog;

--
-- Name: armor_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY armor
    ADD CONSTRAINT armor_pkey PRIMARY KEY (arm_itemid);


--
-- Name: attribute_packages_attr_id_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY attribute_packages
    ADD CONSTRAINT attribute_packages_attr_id_key UNIQUE (attr_id);


--
-- Name: attribute_packages_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY attribute_packages
    ADD CONSTRAINT attribute_packages_pkey PRIMARY KEY (attr_id);


--
-- Name: char_log_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY char_log
    ADD CONSTRAINT char_log_pkey PRIMARY KEY (cl_id);


--
-- Name: chars_pk; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY chars
    ADD CONSTRAINT chars_pk PRIMARY KEY (chr_playerid);


--
-- Name: common_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY common
    ADD CONSTRAINT common_pkey PRIMARY KEY (com_itemid);


--
-- Name: container_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY container
    ADD CONSTRAINT container_pkey PRIMARY KEY (con_itemid);


--
-- Name: gms_pk; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY gms
    ADD CONSTRAINT gms_pk PRIMARY KEY (gm_charid);


--
-- Name: introduction_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY introduction
    ADD CONSTRAINT introduction_pkey PRIMARY KEY (intro_player, intro_known_player);


--
-- Name: itemname_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY itemname
    ADD CONSTRAINT itemname_pkey PRIMARY KEY (itn_itemid);


--
-- Name: longtimeeffects_lte_effectname_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY longtimeeffects
    ADD CONSTRAINT longtimeeffects_lte_effectname_key UNIQUE (lte_effectname);


--
-- Name: longtimeeffects_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY longtimeeffects
    ADD CONSTRAINT longtimeeffects_pkey PRIMARY KEY (lte_effectid);


--
-- Name: monster_attributes_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY monster_attributes
    ADD CONSTRAINT monster_attributes_pkey PRIMARY KEY (mobattr_monsterid, mobattr_name);


--
-- Name: monster_items_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY monster_items
    ADD CONSTRAINT monster_items_pkey PRIMARY KEY (mobit_monsterid, mobit_position);


--
-- Name: monster_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY monster
    ADD CONSTRAINT monster_pkey PRIMARY KEY (mob_monsterid);


--
-- Name: monster_skills_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY monster_skills
    ADD CONSTRAINT monster_skills_pkey PRIMARY KEY (mobsk_monsterid, mobsk_skill_id);


--
-- Name: monsterattack_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY monsterattack
    ADD CONSTRAINT monsterattack_pkey PRIMARY KEY (mat_race_type);


--
-- Name: naturalarmor_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY naturalarmor
    ADD CONSTRAINT naturalarmor_pkey PRIMARY KEY (nar_race);


--
-- Name: npc_npc_posx_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY npc
    ADD CONSTRAINT npc_npc_posx_key UNIQUE (npc_posx, npc_posy, npc_posz);


--
-- Name: npc_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY npc
    ADD CONSTRAINT npc_pkey PRIMARY KEY (npc_id);


--
-- Name: onlineplayer_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY onlineplayer
    ADD CONSTRAINT onlineplayer_pkey PRIMARY KEY (on_playerid);


--
-- Name: player_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY player
    ADD CONSTRAINT player_pkey PRIMARY KEY (ply_playerid);


--
-- Name: playeritem_datavalues_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY playeritem_datavalues
    ADD CONSTRAINT playeritem_datavalues_pkey PRIMARY KEY (idv_playerid, idv_linenumber, idv_key);


--
-- Name: playeritems_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY playeritems
    ADD CONSTRAINT playeritems_pkey PRIMARY KEY (pit_playerid, pit_linenumber);

ALTER TABLE playeritems CLUSTER ON playeritems_pkey;


--
-- Name: playerlteffectvalues_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY playerlteffectvalues
    ADD CONSTRAINT playerlteffectvalues_pkey PRIMARY KEY (pev_playerid, pev_effectid, pev_name);


--
-- Name: playerskills_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY playerskills
    ADD CONSTRAINT playerskills_pkey PRIMARY KEY (psk_playerid, psk_skill_id);


--
-- Name: primary_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY playerlteffects
    ADD CONSTRAINT primary_key PRIMARY KEY (plte_playerid, plte_effectid);


--
-- Name: questprogress_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY questprogress
    ADD CONSTRAINT questprogress_pkey PRIMARY KEY (qpg_userid, qpg_questid);


--
-- Name: quests_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY quests
    ADD CONSTRAINT quests_pkey PRIMARY KEY (qst_id);


--
-- Name: raceattr_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY raceattr
    ADD CONSTRAINT raceattr_pkey PRIMARY KEY (id);


--
-- Name: scheduledscripts_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY scheduledscripts
    ADD CONSTRAINT scheduledscripts_pkey PRIMARY KEY (sc_scriptname, sc_functionname);


--
-- Name: scriptvariables_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY scriptvariables
    ADD CONSTRAINT scriptvariables_pkey PRIMARY KEY (svt_ids);


--
-- Name: skillgroups_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY skillgroups
    ADD CONSTRAINT skillgroups_pkey PRIMARY KEY (skg_group_id);


--
-- Name: skills_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY skills
    ADD CONSTRAINT skills_pkey PRIMARY KEY (skl_skill_id);


--
-- Name: skl_name_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY skills
    ADD CONSTRAINT skl_name_key UNIQUE (skl_name);


--
-- Name: spawnpoint_monster_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY spawnpoint_monster
    ADD CONSTRAINT spawnpoint_monster_pkey PRIMARY KEY (spm_id, spm_race);


--
-- Name: spawnpoint_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY spawnpoint
    ADD CONSTRAINT spawnpoint_pkey PRIMARY KEY (spp_id);


--
-- Name: spawnpoint_spp_x_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY spawnpoint
    ADD CONSTRAINT spawnpoint_spp_x_key UNIQUE (spp_x, spp_y, spp_z);


--
-- Name: spells_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY spells
    ADD CONSTRAINT spells_pkey PRIMARY KEY (spl_spellid, spl_magictype);


--
-- Name: startpack_items_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY startpack_items
    ADD CONSTRAINT startpack_items_pkey PRIMARY KEY (spi_id, spi_linenumber);


--
-- Name: startpack_skills_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY startpack_skills
    ADD CONSTRAINT startpack_skills_pkey PRIMARY KEY (sps_id, sps_skill_id);


--
-- Name: startpacks_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY startpacks
    ADD CONSTRAINT startpacks_pkey PRIMARY KEY (stp_id);


--
-- Name: startpacks_stp_english_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY startpacks
    ADD CONSTRAINT startpacks_stp_english_key UNIQUE (stp_english);


--
-- Name: startpacks_stp_german_key; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY startpacks
    ADD CONSTRAINT startpacks_stp_german_key UNIQUE (stp_german);


--
-- Name: tiles_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY tiles
    ADD CONSTRAINT tiles_pkey PRIMARY KEY (til_id);


--
-- Name: tilesmodificators_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY tilesmodificators
    ADD CONSTRAINT tilesmodificators_pkey PRIMARY KEY (tim_itemid);


--
-- Name: triggerfields_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY triggerfields
    ADD CONSTRAINT triggerfields_pkey PRIMARY KEY (tgf_posx, tgf_posy, tgf_posz);


--
-- Name: unique_name; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY chars
    ADD CONSTRAINT unique_name UNIQUE (chr_name);


--
-- Name: weapon_pkey; Type: CONSTRAINT; Schema: server; Owner: -; Tablespace: 
--

ALTER TABLE ONLY weapon
    ADD CONSTRAINT weapon_pkey PRIMARY KEY (wp_itemid);


SET search_path = accounts, pg_catalog;

--
-- Name: name_password_index; Type: INDEX; Schema: accounts; Owner: -; Tablespace: 
--

CREATE UNIQUE INDEX name_password_index ON account USING btree (acc_login, acc_passwd);


SET search_path = server, pg_catalog;

--
-- Name: mobit_monster_idx; Type: INDEX; Schema: server; Owner: -; Tablespace: 
--

CREATE INDEX mobit_monster_idx ON monster_items USING btree (mobit_monsterid);


--
-- Name: playeritems_searchindex; Type: INDEX; Schema: server; Owner: -; Tablespace: 
--

CREATE INDEX playeritems_searchindex ON playeritems USING btree (pit_playerid);


--
-- Name: playerskills_searchindex; Type: INDEX; Schema: server; Owner: -; Tablespace: 
--

CREATE INDEX playerskills_searchindex ON playerskills USING btree (psk_playerid);


--
-- Name: gmrights_update; Type: RULE; Schema: server; Owner: -
--

CREATE RULE gmrights_update AS ON UPDATE TO gmrights DO INSTEAD UPDATE gms SET gm_rights_server = ((((((((((((((((cast_bool(new.allow_login) + (cast_bool(new.basic_commands) << 1)) + (cast_bool(new.warp) << 2)) + (cast_bool(new.summon) << 3)) + (cast_bool(new.prison) << 4)) + (cast_bool(new.settiles) << 5)) + (cast_bool(new.clipping) << 6)) + (cast_bool(new.warp_field_edit) << 7)) + (cast_bool(new.import) << 8)) + (cast_bool(new.visibility) << 9)) + (cast_bool(new.table_reload) << 10)) + (cast_bool(new.ban) << 11)) + (cast_bool(new.toggle_login) << 12)) + (cast_bool(new.save) << 13)) + (cast_bool(new.broadcast) << 14)) + (cast_bool(new.forcelogout) << 15)) + (cast_bool(new.receive_gmcalls) << 16)) WHERE ((gms.gm_login)::text = (new.gm_login)::text);


SET search_path = accounts, pg_catalog;

--
-- Name: acc_id_1_account_key; Type: FK CONSTRAINT; Schema: accounts; Owner: -
--

ALTER TABLE ONLY legtimulti
    ADD CONSTRAINT acc_id_1_account_key FOREIGN KEY (acc_id_1) REFERENCES account(acc_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: acc_id_2_account_key; Type: FK CONSTRAINT; Schema: accounts; Owner: -
--

ALTER TABLE ONLY legtimulti
    ADD CONSTRAINT acc_id_2_account_key FOREIGN KEY (acc_id_2) REFERENCES account(acc_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: account_groups_ag_acc_id_fkey; Type: FK CONSTRAINT; Schema: accounts; Owner: -
--

ALTER TABLE ONLY account_groups
    ADD CONSTRAINT account_groups_ag_acc_id_fkey FOREIGN KEY (ag_acc_id) REFERENCES account(acc_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: account_groups_ag_group_id_fkey; Type: FK CONSTRAINT; Schema: accounts; Owner: -
--

ALTER TABLE ONLY account_groups
    ADD CONSTRAINT account_groups_ag_group_id_fkey FOREIGN KEY (ag_group_id) REFERENCES groups(g_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: account_log_al_gm_id_fkey; Type: FK CONSTRAINT; Schema: accounts; Owner: -
--

ALTER TABLE ONLY account_log
    ADD CONSTRAINT account_log_al_gm_id_fkey FOREIGN KEY (al_gm_id) REFERENCES account(acc_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: account_log_al_user_id_fkey; Type: FK CONSTRAINT; Schema: accounts; Owner: -
--

ALTER TABLE ONLY account_log
    ADD CONSTRAINT account_log_al_user_id_fkey FOREIGN KEY (al_user_id) REFERENCES account(acc_id) ON UPDATE CASCADE ON DELETE CASCADE;


SET search_path = server, pg_catalog;

--
-- Name: $1; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY container
    ADD CONSTRAINT "$1" FOREIGN KEY (con_itemid) REFERENCES common(com_itemid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: $1; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY itemname
    ADD CONSTRAINT "$1" FOREIGN KEY (itn_itemid) REFERENCES common(com_itemid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: $1; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY monster_skills
    ADD CONSTRAINT "$1" FOREIGN KEY (mobsk_monsterid) REFERENCES monster(mob_monsterid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: $1; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playerskills
    ADD CONSTRAINT "$1" FOREIGN KEY (psk_playerid) REFERENCES player(ply_playerid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: $1; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY gmpager
    ADD CONSTRAINT "$1" FOREIGN KEY (pager_user) REFERENCES player(ply_playerid) MATCH FULL ON UPDATE CASCADE ON DELETE CASCADE DEFERRABLE;


--
-- Name: $2; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY monster_attributes
    ADD CONSTRAINT "$2" FOREIGN KEY (mobattr_monsterid) REFERENCES monster(mob_monsterid) MATCH FULL ON DELETE CASCADE;


--
-- Name: $2; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY monster_items
    ADD CONSTRAINT "$2" FOREIGN KEY (mobit_monsterid) REFERENCES monster(mob_monsterid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: $3; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY monster_items
    ADD CONSTRAINT "$3" FOREIGN KEY (mobit_itemid) REFERENCES common(com_itemid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: account_id; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY chars
    ADD CONSTRAINT account_id FOREIGN KEY (chr_accid) REFERENCES accounts.account(acc_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: armor_arm_itemid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY armor
    ADD CONSTRAINT armor_arm_itemid_fkey FOREIGN KEY (arm_itemid) REFERENCES common(com_itemid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: common_com_objectafterrot_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY common
    ADD CONSTRAINT common_com_objectafterrot_fkey FOREIGN KEY (com_objectafterrot) REFERENCES common(com_itemid) MATCH FULL ON DELETE RESTRICT;


--
-- Name: deleted_chars_dc_acc_id_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY deleted_chars
    ADD CONSTRAINT deleted_chars_dc_acc_id_fkey FOREIGN KEY (dc_acc_id) REFERENCES accounts.account(acc_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: effect_avaiable; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playerlteffectvalues
    ADD CONSTRAINT effect_avaiable FOREIGN KEY (pev_playerid, pev_effectid) REFERENCES playerlteffects(plte_playerid, plte_effectid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: gmpager_pager_gm_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY gmpager
    ADD CONSTRAINT gmpager_pager_gm_fkey FOREIGN KEY (pager_gm) REFERENCES gms(gm_charid) MATCH FULL ON UPDATE CASCADE ON DELETE SET NULL DEFERRABLE;


--
-- Name: gms_gm_charid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY gms
    ADD CONSTRAINT gms_gm_charid_fkey FOREIGN KEY (gm_charid) REFERENCES chars(chr_playerid) MATCH FULL ON UPDATE CASCADE ON DELETE CASCADE DEFERRABLE;


--
-- Name: introduction_intro_known_player_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY introduction
    ADD CONSTRAINT introduction_intro_known_player_fkey FOREIGN KEY (intro_known_player) REFERENCES player(ply_playerid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: introduction_intro_player_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY introduction
    ADD CONSTRAINT introduction_intro_player_fkey FOREIGN KEY (intro_player) REFERENCES player(ply_playerid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: mobsk_skill_id_key; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY monster_skills
    ADD CONSTRAINT mobsk_skill_id_key FOREIGN KEY (mobsk_skill_id) REFERENCES skills(skl_skill_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: onlineplayer_on_playerid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY onlineplayer
    ADD CONSTRAINT onlineplayer_on_playerid_fkey FOREIGN KEY (on_playerid) REFERENCES player(ply_playerid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: player; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playerlteffects
    ADD CONSTRAINT player FOREIGN KEY (plte_playerid) REFERENCES player(ply_playerid) MATCH FULL ON DELETE CASCADE DEFERRABLE;


--
-- Name: player_ply_playerid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY player
    ADD CONSTRAINT player_ply_playerid_fkey FOREIGN KEY (ply_playerid) REFERENCES chars(chr_playerid) MATCH FULL ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: playeritem_datavalues_idv_playerid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playeritem_datavalues
    ADD CONSTRAINT playeritem_datavalues_idv_playerid_fkey FOREIGN KEY (idv_playerid, idv_linenumber) REFERENCES playeritems(pit_playerid, pit_linenumber) MATCH FULL ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: playeritems_pit_itemid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playeritems
    ADD CONSTRAINT playeritems_pit_itemid_fkey FOREIGN KEY (pit_itemid) REFERENCES common(com_itemid) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: playeritems_pit_playerid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playeritems
    ADD CONSTRAINT playeritems_pit_playerid_fkey FOREIGN KEY (pit_playerid) REFERENCES player(ply_playerid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: playerlteffects_plte_effectid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playerlteffects
    ADD CONSTRAINT playerlteffects_plte_effectid_fkey FOREIGN KEY (plte_effectid) REFERENCES longtimeeffects(lte_effectid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: psk_skill_id_key; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY playerskills
    ADD CONSTRAINT psk_skill_id_key FOREIGN KEY (psk_skill_id) REFERENCES skills(skl_skill_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: questprogress_qpg_userid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY questprogress
    ADD CONSTRAINT questprogress_qpg_userid_fkey FOREIGN KEY (qpg_userid) REFERENCES player(ply_playerid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: skl_group_key; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY skills
    ADD CONSTRAINT skl_group_key FOREIGN KEY (skl_group_id) REFERENCES skillgroups(skg_group_id) ON DELETE CASCADE;


--
-- Name: spawnpoint_monster_spm_id_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY spawnpoint_monster
    ADD CONSTRAINT spawnpoint_monster_spm_id_fkey FOREIGN KEY (spm_id) REFERENCES spawnpoint(spp_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: spawnpoint_monster_spm_race_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY spawnpoint_monster
    ADD CONSTRAINT spawnpoint_monster_spm_race_fkey FOREIGN KEY (spm_race) REFERENCES monster(mob_monsterid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: startpack_items_id_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY startpack_items
    ADD CONSTRAINT startpack_items_id_fkey FOREIGN KEY (spi_id) REFERENCES startpacks(stp_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: startpack_items_item_id_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY startpack_items
    ADD CONSTRAINT startpack_items_item_id_fkey FOREIGN KEY (spi_item_id) REFERENCES common(com_itemid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: startpack_skills_id_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY startpack_skills
    ADD CONSTRAINT startpack_skills_id_fkey FOREIGN KEY (sps_id) REFERENCES startpacks(stp_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: startpack_skills_skill_id_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY startpack_skills
    ADD CONSTRAINT startpack_skills_skill_id_fkey FOREIGN KEY (sps_skill_id) REFERENCES skills(skl_skill_id) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: status_gm; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY chars
    ADD CONSTRAINT status_gm FOREIGN KEY (chr_statusgm) REFERENCES gms(gm_charid) ON DELETE SET NULL;


--
-- Name: tilesmodificators_tim_itemid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY tilesmodificators
    ADD CONSTRAINT tilesmodificators_tim_itemid_fkey FOREIGN KEY (tim_itemid) REFERENCES common(com_itemid) ON UPDATE CASCADE ON DELETE CASCADE;


--
-- Name: weapon_wp_itemid_fkey; Type: FK CONSTRAINT; Schema: server; Owner: -
--

ALTER TABLE ONLY weapon
    ADD CONSTRAINT weapon_wp_itemid_fkey FOREIGN KEY (wp_itemid) REFERENCES common(com_itemid) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- PostgreSQL database dump complete
--


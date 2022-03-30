///import 'dart:math' as math;
///import 'package:http/http.dart' as http;
import 'dart:convert';
import 'package:mongo_dart/mongo_dart.dart';
import 'package:ansicolor/ansicolor.dart';
import 'dart:io';
import 'package:logging/logging.dart';
import 'package:json_sorter/json_sorter.dart';

/// A class to access the MongoDb database
///
/// It is used bothe to store process configuration and
/// asics slowcontrol value
///
///
class MongoAccess {
  /// [dbname] MongoDb database name
  final dbName = "LYONROC";
  late String account;
  late Db db;
  final _log = Logger('MongoAccess');

  /// Constructor
  /// [ac] is the account define by user/pwd@host:port@dbname
  MongoAccess(String ac) {
    account = ac;
    db = new Db(account);
  }

  /// database access open
  Future<void> open() async {
    await db.open();
    _log.fine(''' Db is Open ''');
  }

  /// database access close
  Future<void> close() async {
    db.close();
    _log.fine(''' Db is Closed ''');
  }

  /// List all process  configurations in the DB
  Future<void> listConfigurations() async {
    AnsiPen pen = new AnsiPen()..blue(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('configurations');
    var conf = await coll.find().toList();

    for (var v in conf) {
      num fmstoepoch = v['time'] * 1.0E6;
      int mst = fmstoepoch.ceil();
      print(redTextBlueBackgroundPen(" ${v['name']} ${v['version']}") +
          "  ${DateTime.fromMicrosecondsSinceEpoch(mst)}" +
          pen(" ${v['comment']}"));
    }
  }

  ///List all runs in the DB ,
  ///
  ///if [location] is set only runs from this setup are shown
  Future<void> listRuns({String location = ""}) async {
    AnsiPen pen = new AnsiPen()..blue(bold: true);
    AnsiPen gpen = new AnsiPen()..green(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('runs');
    var sf = new Map<String, dynamic>();
    if (location != "") {
      sf["location"] = location;
    }
    var conf = await coll.find(sf).toList();

    for (var v in conf) {
      num fmstoepoch = v['time'] * 1.0E6;
      int mst = fmstoepoch.ceil();
      //print("${fmstoepoch} ${mst}");
      //for (var x in v.keys) {
      //  if (x != "_id") print("${x} ${v[x]}");
      // }
      //print("${v}");
      String sr = redTextBlueBackgroundPen(" ${v['run']} ${v['location']}") +
          "  ${DateTime.fromMicrosecondsSinceEpoch(mst)}" +
          pen(" ${v['comment']}");
      /* for (var x in v.keys) {
        if (x == "P") sr = sr + gpen(" P=${v[x]}");
      } */
      print(sr);
    }
  }

  ///Print the information for a given [run] in a given setup([location])
  ///
  Future<void> runInfo(String location, int run) async {
    AnsiPen pen = new AnsiPen()..blue(bold: true);
    AnsiPen gpen = new AnsiPen()..green(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('runs');
    var sf = new Map<String, dynamic>();
    sf["location"] = location;
    sf["run"] = run;
    var conf =
        await coll.find(where.eq('run', run).eq('location', location)).toList();

    for (var v in conf) {
      num fmstoepoch = v['time'] * 1.0E6;
      int mst = fmstoepoch.ceil();
      //print("${fmstoepoch} ${mst}");
      //for (var x in v.keys) {
      //  if (x != "_id") print("${x} ${v[x]}");
      // }
      //print("${v}");
      String sr = redTextBlueBackgroundPen(" ${v['run']} ${v['location']}") +
          "  ${DateTime.fromMicrosecondsSinceEpoch(mst)}" +
          pen(" ${v['comment']}");
      print(sr);
      for (var x in v.keys) {
        if (x == "_id") continue;
        if (x == "run") continue;
        if (x == "location") continue;
        if (x == "time") continue;
        if (x == "comment") continue;

        print(gpen(" ${x} -> ${v[x]}"));
      }
    }
  }

  /// Update a run entry
  ///
  /// for a given [location] and [run] with a String [tag] and [vtag] value
  ///
  Future<void> updateRun(
      String location, int run, String tag, String vtag) async {
    var coll = db.collection('runs');
    await coll.updateOne(
        where.eq('run', run).eq('location', location), modify.set(tag, vtag));
  }

/*
Download a process configuration with its name and version
The json file is stored in /dev/shm/mgjob/${name}_${version}.json
*/
  Future<void> downloadConfiguration(String name, int version) async {
    var directory = await Directory('/dev/shm/mgjob').create(recursive: true);

    String path = '/dev/shm/mgjob/${name}_${version}.json';

    if (FileSystemEntity.typeSync(path) != FileSystemEntityType.notFound) {
      _log.info("${path} already exists");
      return;
    }
    var coll = db.collection('configurations');

    var conf = await coll
        .find(where.match("name", name).and(where.eq("version", version)))
        .toList();
    //print("${name} ${version} ${conf}");
    for (var v in conf) {
      JsonEncoder encoder = new JsonEncoder.withIndent('  ');
      String s = encoder.convert(v['content']);
      File f = new File(path);
      f.writeAsString(s);
    }
    //conf.forEach((v) => print(json.encode(v['content'])));
  }

/*
upload a process configuration with its name , a json file  and a comment optionnal version set to 1
*/
  Future<void> uploadConfiguration(String fname, String comment) async {
    if (FileSystemEntity.typeSync(fname) == FileSystemEntityType.notFound) {
      _log.severe("${fname} does not exist");
      return;
    }
    var s = new Map<String, dynamic>();
    s["content"] = json.decode(await new File(fname).readAsString());
    if (!s["content"].containsKey("session")) {
      _log.severe("${fname} does not contain session tag");
      return;
    }
    if (!s["content"].containsKey("version")) {
      _log.severe("${fname} does not contain version tag");
      return;
    }
    s["name"] = s["content"]["session"];
    s["comment"] = comment;
    s["version"] = s["content"]["version"];
    s["time"] = DateTime.now().millisecondsSinceEpoch / 1000;
    print(s);

    var coll = db.collection('configurations');

    var ret = await coll.insertOne(s);
    if (!ret.isSuccess) {
      print('Error detected in record insertion');
    }

    //var res = await coll.findOne();

    //print('Fetched ${res?['name']}');
    //conf.forEach((v) => print(json.encode(v['content'])));
  }

  /// Create a new run in the db.runs table
  /// location: setup and comment are compulsory
  ///
  Future<int> getRun(String location, String comment) async {
    var coll = db.collection('runs');

    var conf = await coll.find(where.match("location", location)).toList();
    //print("${name} ${version} ${conf}");
    var lastRun = new Map<String, dynamic>();
    var newRun = new Map<String, dynamic>();
    newRun['location'] = location;
    newRun['comment'] = comment;
    newRun['time'] = (DateTime.now().millisecondsSinceEpoch / 1000);
    for (var v in conf) if (v.containsKey('run')) lastRun = v;
    if (lastRun.length == 0) {
      newRun['run'] = 1000;
    } else {
      newRun['run'] = lastRun['run'] + 1;
    }
    await coll.insert(newRun, writeConcern: WriteConcern.ACKNOWLEDGED);

    return newRun['run'];
  }

  // List Asic configurations
  Future<void> listStates() async {
    AnsiPen pen = new AnsiPen()..blue(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('states');
    var conf = await coll.find().toList();
    //print(conf);
    for (var v in conf) {
      print(redTextBlueBackgroundPen(" ${v['name']} ${v['version']}") +
          pen(" ${v['comment']}"));
      //   print(v['asics']);
    }
  }

  // Download Asic configurations
  Future<void> downloadState(String state, int version) async {
    var directory = await Directory('/dev/shm/mgroc').create(recursive: true);
    //print(directory.path);
    String path = '/dev/shm/mgroc/${state}_${version}.json';

    if (FileSystemEntity.typeSync(path) != FileSystemEntityType.notFound) {
      _log.info("${path} already exists");
      return;
    }
    AnsiPen pen = new AnsiPen()..blue(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('states');
    var cola = db.collection('asics');

    var s = Map<String, dynamic>();
    s["name"] = state;
    s["version"] = version;
    var conf = await coll.find(s).toList();
    //print(conf);
    var slc = new Map<String, dynamic>();
    slc["state"] = state;
    slc["version"] = version;
    slc["asics"] = List.empty(growable: true);
    for (var v in conf) {
      print(redTextBlueBackgroundPen(" ${v['name']} ${v['version']}") +
          pen(" ${v['comment']}"));
      print(" Number of asics ${v['asics'].length}");
      var asconf = await cola.find(where.oneFrom('_id', v['asics'])).toList();

      //print(asconf);
      for (var x in asconf) {
        //print('${x["dif"]} ${x["num"]}');
        var sa = Map<String, dynamic>();
        sa["slc"] = x["slc"];
        sa["num"] = x["num"];
        sa["dif"] = x["dif"];
        if (x.containsKey("address")) sa["address"] = x["address"];
        slc["asics"].add(sa);
      }
      //print(slc);

      JsonEncoder encoder = new JsonSortedEncoder(); //.withIndent('  ');
      String sall = encoder.convert(slc);
      File f = new File(path);
      f.writeAsString(sall);
    }
    // var oneasic = buildPR2(12);
    // print(oneasic);
  }

  Map<String, dynamic> buildHR2(int num, {int gain = 128}) {
    var _jasic = new Map<String, dynamic>();
    _jasic["ENABLED"] = 1;
    _jasic["HEADER"] = num;
    _jasic["QSCSROUTSC"] = 1;
    _jasic["ENOCDOUT1B"] = 1;
    _jasic["ENOCDOUT2B"] = 0;
    _jasic["ENOCTRANSMITON1B"] = 1;
    _jasic["ENOCTRANSMITON2B"] = 0;
    _jasic["ENOCCHIPSATB"] = 1;
    _jasic["SELENDREADOUT"] = 1;
    _jasic["SELSTARTREADOUT"] = 1;
    _jasic["CLKMUX"] = 1;
    _jasic["SCON"] = 0;
    _jasic["RAZCHNEXTVAL"] = 0;
    _jasic["RAZCHNINTVAL"] = 1;
    _jasic["TRIGEXTVAL"] = 0;
    _jasic["DISCROROR"] = 1;
    _jasic["ENTRIGOUT"] = 1;
    _jasic["TRIG0B"] = 1;
    _jasic["TRIG1B"] = 0;
    _jasic["TRIG2B"] = 0;
    _jasic["OTABGSW"] = 0;
    _jasic["DACSW"] = 0;
    _jasic["SMALLDAC"] = 0;
    _jasic["B2"] = 250;
    _jasic["B1"] = 250;
    _jasic["B0"] = 250;
    var mask = List.filled(64, 1);

    _jasic["MASK0"] = mask;
    _jasic["MASK1"] = mask;
    _jasic["MASK2"] = mask;
    _jasic["RS_OR_DISCRI"] = 1;
    _jasic["DISCRI1"] = 0;
    _jasic["DISCRI2"] = 0;
    _jasic["DISCRI0"] = 0;
    _jasic["OTAQ_PWRADC"] = 0;
    _jasic["EN_OTAQ"] = 1;
    _jasic["SW50F0"] = 1;
    _jasic["SW100F0"] = 1;
    _jasic["SW100K0"] = 1;
    _jasic["SW50K0"] = 1;
    _jasic["PWRONFSB1"] = 0;
    _jasic["PWRONFSB2"] = 0;
    _jasic["PWRONFSB0"] = 0;
    _jasic["SEL1"] = 0;
    _jasic["SEL0"] = 1;
    _jasic["SW50F1"] = 1;
    _jasic["SW100F1"] = 1;
    _jasic["SW100K1"] = 1;
    _jasic["SW50K1"] = 1;
    _jasic["CMDB0FSB1"] = 1;
    _jasic["CMDB1FSB1"] = 1;
    _jasic["CMDB2FSB1"] = 0;
    _jasic["CMDB3FSB1"] = 1;
    _jasic["SW50F2"] = 1;
    _jasic["SW100F2"] = 1;
    _jasic["SW100K2"] = 1;
    _jasic["SW50K2"] = 1;
    _jasic["CMDB0FSB2"] = 1;
    _jasic["CMDB1FSB2"] = 1;
    _jasic["CMDB2FSB2"] = 0;
    _jasic["CMDB3FSB2"] = 1;
    _jasic["PWRONW"] = 0;
    _jasic["PWRONSS"] = 0;
    _jasic["PWRONBUFF"] = 0;
    _jasic["SWSSC"] = 7;
    _jasic["CMDB0SS"] = 0;
    _jasic["CMDB1SS"] = 0;
    _jasic["CMDB2SS"] = 0;
    _jasic["CMDB3SS"] = 0;
    _jasic["PWRONPA"] = 0;
    //Unset power pulsing
    _jasic["CLKMUX"] = 1;
    _jasic["SCON"] = 1;
    _jasic["OTAQ_PWRADC"] = 1;
    _jasic["PWRONW"] = 1;
    _jasic["PWRONSS"] = 0;
    _jasic["PWRONBUFF"] = 1;
    _jasic["PWRONPA"] = 1;
    _jasic["DISCRI0"] = 1;
    _jasic["DISCRI1"] = 1;
    _jasic["DISCRI2"] = 1;
    _jasic["OTABGSW"] = 1;
    _jasic["DACSW"] = 1;
    _jasic["PWRONFSB0"] = 1;
    _jasic["PWRONFSB1"] = 1;
    _jasic["PWRONFSB2"] = 1;

    var dv = List<int>.filled(32, gain);
    var ct = List<int>.filled(64, 0);

    _jasic["PAGAIN"] = dv;
    _jasic["CTEST"] = ct;

    return _jasic;
  }

  Map<String, dynamic> buildPR2(int num, {String version = "PR2B"}) {
    var _jasic = new Map<String, dynamic>();
    _jasic["header"] = num;
    _jasic["EN10bDac"] = 1;
    _jasic["PP10bDac"] = 0;
    _jasic["EN_adc"] = 0;
    _jasic["PP_adc"] = 0;
    _jasic["sel_starb_ramp_adc_ext"] = 0;
    _jasic["usebcompensation"] = 0;
    _jasic["EN_bias_dac_delay"] = 0;
    _jasic["PP_bias_dac_delay"] = 0;
    _jasic["EN_bias_ramp_delay"] = 0;
    _jasic["PP_bias_ramp_delay"] = 0;
    _jasic["EN_discri_delay"] = 0;
    _jasic["PP_discri_delay"] = 0;
    _jasic["EN_temp_sensor"] = 0;
    _jasic["PP_temp_sensor"] = 0;
    _jasic["EN_bias_pa"] = 1;
    _jasic["PP_bias_pa"] = 0;
    _jasic["EN_bias_discri"] = 1;
    _jasic["PP_bias_discri"] = 0;
    _jasic["cmd_polarity"] = 0;
    _jasic["latch"] = 1;
    _jasic["EN_bias_6bit_dac"] = 1;
    _jasic["PP_bias_6bit_dac"] = 0;
    _jasic["EN_bias_tdc"] = 0;
    _jasic["PP_bias_tdc"] = 0;
    _jasic["ON_OFF_input_dac"] = 1;
    _jasic["EN_bias_charge"] = 0;
    _jasic["PP_bias_charge"] = 0;
    _jasic["Cf3_100fF"] = 0;
    _jasic["Cf2_200fF"] = 0;
    _jasic["Cf1_2p5pF"] = 0;
    _jasic["Cf0_1p25pF"] = 0;
    _jasic["EN_bias_sca"] = 0;
    _jasic["PP_bias_sca"] = 0;
    _jasic["EN_bias_discri_charge"] = 0;
    _jasic["PP_bias_discri_charge"] = 0;
    _jasic["EN_bias_discri_adc_time"] = 0;
    _jasic["PP_bias_discri_adc_time"] = 0;
    _jasic["EN_bias_discri_adc_charge"] = 0;
    _jasic["PP_bias_discri_adc_charge"] = 0;
    _jasic["DIS_razchn_int"] = 1;
    _jasic["DIS_razchn_ext"] = 0;
    _jasic["SEL_80M"] = 0;
    _jasic["EN_80M"] = 0;
    _jasic["EN_slow_lvds_rec"] = 1;
    _jasic["PP_slow_lvds_rec"] = 0;
    _jasic["EN_fast_lvds_rec"] = 1;
    _jasic["PP_fast_lvds_rec"] = 0;
    _jasic["EN_transmitter"] = 0;
    _jasic["PP_transmitter"] = 0;
    _jasic["ON_OFF_1mA"] = 1;
    _jasic["ON_OFF_2mA"] = 1;
    _jasic["ON_OFF_otaQ"] = 0;
    _jasic["ON_OFF_ota_mux"] = 0;
    _jasic["ON_OFF_ota_probe"] = 0;
    _jasic["DIS_trig_mux"] = 1;
    _jasic["EN_NOR32_time"] = 1;
    _jasic["EN_NOR32_charge"] = 0;
    _jasic["DIS_triggers"] = 0;
    _jasic["EN_dout_oc"] = 0;
    _jasic["EN_transmit"] = 1;
    if (version == "PR2B") {
      _jasic["PA_ccomp_0"] = 0;
      _jasic["PA_ccomp_1"] = 0;
      _jasic["PA_ccomp_2"] = 0;
      _jasic["PA_ccomp_3"] = 0;
      _jasic["Choice_Trigger_Out"] = 0;
    }
    _jasic["DacDelay"] = 0;
    var idac = List<int>.filled(32, 125);
    var bdac = List<int>.filled(32, 31);
    var mdc = List<int>.filled(32, 1);
    var mdt = List<int>.filled(32, 0);
    var idc = List<int>.filled(32, 1);
    _jasic["InputDac"] = idac;
    ;
    _jasic["6bDac"] = bdac;
    ;
    _jasic["MaskDiscriCharge"] = mdc;
    ;
    _jasic["MaskDiscriTime"] = mdt;
    ;
    _jasic["InputDacCommand"] = idc;
    ;

    _jasic["VthDiscriCharge"] = 863;
    _jasic["VthTime"] = 610;

    return _jasic;
  }
}

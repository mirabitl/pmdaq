import 'package:mgaccess/mgaccess.dart' as mg;
import 'dart:convert';
import 'package:mongo_dart/mongo_dart.dart'
    show Db, WriteConcern, modify, where;
import 'package:ansicolor/ansicolor.dart';
import 'dart:io';
import 'package:logging/logging.dart';
import 'package:json_sorter/json_sorter.dart';

class mgRoc extends mg.MongoAccess {
  final difList = Map<int, dynamic>();
  final asicIds = List.empty(growable: true);
  late String currentStateName;
  late int currentVersion;

  /// Constructor
  mgRoc(String account) : super(account) {
    difList.clear();
    asicIds.clear();
    currentVersion = 0;
    currentStateName = "";
  }

  ///List all states in the DB ,
  ///
  /// if [state] is set list only version of this state
  Future<void> list({String state = ""}) async {
    AnsiPen pen = new AnsiPen()..blue(bold: true);
    AnsiPen gpen = new AnsiPen()..green(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('states');
    var sf = new Map<String, dynamic>();
    if (state != "") {
      sf["name"] = state;
    }
    var conf = await coll.find(sf).toList();

    for (var v in conf) {
      String sr = redTextBlueBackgroundPen(" ${v['name']} ${v['version']}") +
          pen(" ${v['comment']}");
      /* for (var x in v.keys) {
        if (x == "P") sr = sr + gpen(" P=${v[x]}");
      } */
      print(sr);
    }
  }

  ///Print the information for a given [state] in a given [version]
  ///
  Future<void> info(String state, int version) async {
    AnsiPen pen = new AnsiPen()..blue(bold: true);
    AnsiPen gpen = new AnsiPen()..green(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('states');
    var sf = new Map<String, dynamic>();
    sf["name"] = state;
    sf["version"] = version;
    var conf = await coll
        .find(where.eq('name', state).eq('version', version))
        .toList();

    for (var v in conf) {
      String sr = redTextBlueBackgroundPen(" ${v['name']} ${v['version']}") +
          pen(" ${v['comment']}");
      print(sr);
      print('Number of asics: ${v["asics"].length}');
      for (var x in v.keys) {
        if (x == "_id") continue;
        if (x == "name") continue;
        if (x == "version") continue;
        if (x == "comment") continue;
        if (x == "asics") continue;

        print(gpen(" ${x} -> ${v[x]}"));
      }
    }
  }

  /// Last version of a given state
  ///
  ///
  Future<int> lastVersion(String state) async {
    var coll = db.collection('states');
    var conf = await coll.find(where.eq('name', state)).toList();
    int last = 1;
    for (var v in conf) {
      if (v["version"] > last) last = v["version"];
    }
    return last;
  }

  /// Memory storage of a given version
  ///
  ///
  Future<void> memoryStore(String state, int version) async {
    var coll = db.collection('states');
    var cola = this.db.collection('asics');
    var conf = await coll
        .find(where.eq('name', state).eq('version', version))
        .toList();
    for (var v in conf) {
      /// Build DIF list
      ///
      difList.clear();
      var asconf = await cola.find(where.oneFrom('_id', v['asics'])).toList();

      for (var x in asconf) {
        //print(x);
        int difid = x["dif"];
        int anum = x["num"];

        if (!difList.containsKey(difid)) {
          //print("adding dif entry ${difList.length}");
          difList[difid] = new Map<int, dynamic>();
        }
        if (x.containsKey("slc") && difList.containsKey(difid)) {
          difList[difid][anum] = x;
          //print("adding asic entry ${difList[difid].length}  ${difList[difid][anum]}");
        }
      }
    }
  }

  ///Print ASIC information for a given [state] in a given [version]
  ///
  Future<void> asicInfo(String state, int version,
      {int wdif = 0, int wasic = 0}) async {
    AnsiPen pen = new AnsiPen()..blue(bold: true);
    AnsiPen gpen = new AnsiPen()..green(bold: true);
    AnsiPen ypen = new AnsiPen()..cyan(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = db.collection('states');
    var cola = this.db.collection('asics');

    var sf = new Map<String, dynamic>();
    sf["name"] = state;
    sf["version"] = version;
    var conf = await coll
        .find(where.eq('name', state).eq('version', version))
        .toList();

    for (var v in conf) {
      String sr = redTextBlueBackgroundPen(" ${v['name']} ${v['version']}") +
          pen(" ${v['comment']}");
      print(sr);
      print('Number of asics: ${v["asics"].length}');
      for (var x in v.keys) {
        if (x == "_id") continue;
        if (x == "name") continue;
        if (x == "version") continue;
        if (x == "comment") continue;
        if (x == "asics") continue;

        print(gpen(" ${x} -> ${v[x]}"));
      }

      /// Build DIF list
      ///
      difList.clear();

      var slc = new Map<String, dynamic>();
      slc["state"] = state;
      slc["version"] = version;
      slc["asics"] = List.empty(growable: true);

      var asconf = await cola.find(where.oneFrom('_id', v['asics'])).toList();

      for (var x in asconf) {
        //print(x);
        int difid = x["dif"];
        int anum = x["num"];

        if (!difList.containsKey(difid)) {
          //print("adding dif entry ${difList.length}");
          difList[difid] = new Map<int, dynamic>();
        }
        if (x.containsKey("slc") && difList.containsKey(difid)) {
          difList[difid][anum] = x;
          //print("adding asic entry ${difList[difid].length}  ${difList[difid][anum]}");
        }
      }

      /// Loop on dif
      ///
      for (var x in difList.keys) {
        if (wdif != 0 && x != wdif) continue;
        print(ypen(" \t DIF ${x} has ${difList[x]?.length} asics"));
        if (wasic != 0 && x == wdif) {
          print(gpen(" ASIC ${wdif} ${wasic}") + pen("${difList[x][wasic]}"));
        }
      }
    }
  }

  /// update a state entry
  ///
  /// for a given [state] and [version] with a String [tag] and [vtag] value
  ///
  Future<void> update(
      String state, int version, String tag, String vtag) async {
    var coll = db.collection('states');
    await coll.updateOne(
        where.eq('name', state).eq('version', version), modify.set(tag, vtag));
  }

  /// Download Asic configurations
  ///
  /// for a given [state] and [version]
  Future<void> download(String state, int version) async {
    var directory = await Directory('/dev/shm/mgroc').create(recursive: true);
    //print(directory.path);
    String path = '/dev/shm/mgroc/${state}_${version}.json';

    if (FileSystemEntity.typeSync(path) != FileSystemEntityType.notFound) {
      log.info("${path} already exists");
      return;
    }
    AnsiPen pen = new AnsiPen()..blue(bold: true);

    AnsiPen redTextBlueBackgroundPen = AnsiPen()
      ..yellow(bg: true)
      ..red();
    var coll = this.db.collection('states');
    var cola = this.db.collection('asics');

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
    _jasic["6bDac"] = bdac;
    _jasic["MaskDiscriCharge"] = mdc;
    _jasic["MaskDiscriTime"] = mdt;
    _jasic["InputDacCommand"] = idc;
    _jasic["VthDiscriCharge"] = 863;
    _jasic["VthTime"] = 610;

    return _jasic;
  }
}

///import 'dart:math' as math;
///import 'package:http/http.dart' as http;
import 'dart:convert';
import 'package:mongo_dart/mongo_dart.dart';
import 'package:ansicolor/ansicolor.dart';
import 'dart:io';
import 'package:logging/logging.dart';

class MongoAccess {
  // field
  final dbName = "LYONROC";
  late String account;
  late Db db;
  final _log = Logger('MongoAccess');
  // Constructor
  MongoAccess(String ac) {
    account = ac;
    db = new Db(account);
  }

  Future<void> open() async {
    await db.open();
    _log.fine(''' Db is Open ''');
  }

  Future<void> close() async {
    db.close();
    _log.fine(''' Db is Closed ''');
  }

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
      for (var x in v.keys) {
        if (x == "P") sr = sr + gpen(" P=${v[x]}");
      }
      print(sr);
    }
  }

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

  Future<void> updateRun(
      String location, int run, String tag, String vtag) async {
    var coll = db.collection('runs');
    await coll.updateOne(
        where.eq('run', run).eq('location', location), modify.set(tag, vtag));
  }

  Future<void> downloadConfiguration(String name, int version) async {
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

  /// Get a run
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
}

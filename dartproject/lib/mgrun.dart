import 'package:mgaccess/mgaccess.dart' as mg;
import 'dart:convert';
import 'package:mongo_dart/mongo_dart.dart'
    show Db, WriteConcern, modify, where;
import 'package:ansicolor/ansicolor.dart';
import 'dart:io';
import 'package:logging/logging.dart';
import 'package:json_sorter/json_sorter.dart';

class mgRun extends mg.MongoAccess {
  /// Constructor
  mgRun(String account) : super(account) {}

  ///List all runs in the DB ,
  ///
  ///if [location] is set only runs from this setup are shown
  Future<void> list({String location = ""}) async {
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
  Future<void> info(String location, int run) async {
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

  /// update a run entry
  ///
  /// for a given [location] and [run] with a String [tag] and [vtag] value
  ///
  Future<void> update(String location, int run, String tag, String vtag) async {
    var coll = db.collection('runs');
    await coll.updateOne(
        where.eq('run', run).eq('location', location), modify.set(tag, vtag));
  }
}

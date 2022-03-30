import 'package:mgaccess/mgaccess.dart' as mg;
import 'dart:convert';
import 'package:mongo_dart/mongo_dart.dart'
    show Db, WriteConcern, modify, where;
import 'package:ansicolor/ansicolor.dart';
import 'dart:io';
import 'package:logging/logging.dart';
import 'package:json_sorter/json_sorter.dart';

class mgConfig extends mg.MongoAccess {
  /// Constructor
  mgConfig(String account) : super(account) {}

  /// List all process  configurations in the DB
  Future<void> list() async {
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

  /// Download a process configuration
//
// with its [name] and [version]
  /// The json file is stored in /dev/shm/mgjob/${name}_${version}.json
  ///
  Future<void> download(String name, int version) async {
    var directory = await Directory('/dev/shm/mgjob').create(recursive: true);

    String path = '/dev/shm/mgjob/${name}_${version}.json';

    if (FileSystemEntity.typeSync(path) != FileSystemEntityType.notFound) {
      log.info("${path} already exists");
      return;
    }
    var coll = this.db.collection('configurations');

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

  /// upload a new configuration
  ///
  /// The name (seesion tag) and version should be specified in the json file [fname]
  /// a [comment] is also required
  ///
  Future<void> upload(String fname, String comment) async {
    if (FileSystemEntity.typeSync(fname) == FileSystemEntityType.notFound) {
      log.severe("${fname} does not exist");
      return;
    }
    var s = new Map<String, dynamic>();
    s["content"] = json.decode(await new File(fname).readAsString());
    if (!s["content"].containsKey("session")) {
      log.severe("${fname} does not contain session tag");
      return;
    }
    if (!s["content"].containsKey("version")) {
      log.severe("${fname} does not contain version tag");
      return;
    }
    s["name"] = s["content"]["session"];
    s["comment"] = comment;
    s["version"] = s["content"]["version"];
    s["time"] = DateTime.now().millisecondsSinceEpoch / 1000;
    print(s);

    var coll = this.db.collection('configurations');

    var ret = await coll.insertOne(s);
    if (!ret.isSuccess) {
      print('Error detected in record insertion');
    }

    //var res = await coll.findOne();

    //print('Fetched ${res?['name']}');
    //conf.forEach((v) => print(json.encode(v['content'])));
  }
}

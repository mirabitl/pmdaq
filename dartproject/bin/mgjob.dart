import 'package:mgaccess/mgaccess.dart' as mg;
import 'package:mgaccess/mgconfig.dart' as mgc;
import 'package:mgaccess/mgrun.dart' as mgr;
import 'package:mgaccess/mgasic.dart' as mgroc;

import 'dart:io';
import 'dart:convert';
import 'package:logging/logging.dart';
import 'package:args/args.dart';

void main(List<String> arguments) async {
  Logger.root.level = Level.INFO; // defaults to Level.INFO
  Logger.root.onRecord.listen((record) {
    print('${record.level.name}: ${record.time}: ${record.message}');
  });
  ArgResults argResults;

  final ArgParser argParser = new ArgParser()
    ..addOption('Mongo',
        abbr: 'm', defaultsTo: Platform.environment['MGDBLOGIN'])
    ..addOption('run', abbr: 'r', defaultsTo: '0')
    ..addOption('setup', defaultsTo: 'NotSet')
    ..addOption('vtag', defaultsTo: '0')
    ..addOption('tag', defaultsTo: 'NotSet')
    ..addOption('version', abbr: 'v', defaultsTo: '0')
    ..addOption('config', defaultsTo: 'NotSet')
    ..addOption('file', defaultsTo: 'NotSet')
    ..addOption('comment', defaultsTo: 'NotSet')
    ..addOption('state', defaultsTo: 'NotSet')
    ..addFlag('CONFIG', abbr: 'C', negatable: false, help: 'Configuration menu')
    ..addFlag('RUN', abbr: 'R', negatable: false, help: 'Run menu')
    ..addFlag('ASICS', abbr: 'A', negatable: false, help: 'Asics menu')
    ..addFlag('LIST', abbr: 'L', negatable: false, help: " List option")
    ..addFlag('DOWNLOAD', abbr: 'D', negatable: false, help: " Download option")
    ..addFlag('UPLOAD', abbr: 'U', negatable: false, help: " upload option")
    ..addFlag('UPDATE',
        abbr: 'M', negatable: false, help: " update/modify option")
    ..addFlag('INFO', abbr: 'I', negatable: false, help: " info option")
    ..addFlag('list-runs',
        negatable: false, help: 'List all the run with optional setup')
    ..addFlag('run-info',
        negatable: false, help: 'runinfo  with non-optional setup and run')
    ..addFlag('update-run',
        negatable: false,
        help:
            'update runinfo  with non-optional setup and run and tag and vtag')
    ..addFlag('list-conf', negatable: false, help: 'List all the configuration')
    ..addFlag('download-conf',
        negatable: false, help: 'runinfo  with non-optional setup and run')
    ..addFlag('upload-conf',
        negatable: false,
        help:
            'update runinfo  with non-optional setup and run and tag and vtag')
    ..addFlag('list-states', negatable: false, help: 'List all the states')
    ..addFlag('download-state',
        negatable: false, help: 'runinfo  with non-optional setup and run')
    ..addFlag('help',
        abbr: 'h', negatable: false, help: "Displays this help information.");
  argResults = argParser.parse(arguments);

  if (argResults['help']) {
    print("""
** HELP **
${argParser.usage}
    """);
    exit(0);
  }

  /// Use flag requests

  mg.MongoAccess _mongoAccess;
  final _log = Logger('daqControl');

  String _account = argResults["Mongo"];
  // "acqilc/RPC_2008@lyocmsmu04:27017@LYONROC";
  String userinfo = _account.split("@")[0];
  String hostinfo = _account.split("@")[1];
  String dbname = _account.split("@")[2];
  String user = userinfo.split("/")[0];
  String pwd = userinfo.split("/")[1];
  String host = hostinfo.split(":")[0];
  int port = int.parse(hostinfo.split(":")[1]);

  //_log.info(" Account is ${user} ${pwd} ${host} ${port} ${dbname}");

  if (argResults["CONFIG"]) {
    var _mgc =
        new mgc.mgConfig("mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    // _mongoAccess = new mg.MongoAccess(
    //     "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mgc.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");

    if (argResults["LIST"]) await _mgc.list();

    if (argResults["DOWNLOAD"]) {
      if (argResults["config"] == "NotSet" || argResults["version"] == '0') {
        _log.severe(" config and version should be specify");
      } else {
        await _mgc.download(
            argResults["config"], int.parse(argResults["version"]));
      }
    }
    if (argResults["UPLOAD"]) {
      if (argResults["file"] == "NotSet" || argResults["comment"] == 'NotSet') {
        _log.severe(" cfile and  comment should be specify");
      } else {
        await _mgc.upload(argResults["file"], argResults["comment"]);
      }
    }

    await _mgc.close();
  }

  if (argResults["ASICS"]) {
    var _mgr =
        new mgroc.mgRoc("mongodb://${user}:${pwd}@${host}:${port}/${dbname}");

    await _mgr.open();

    if (argResults["LIST"]) {
      if (argResults["state"] == "NotSet")
        await _mgr.list();
      else
        await _mgr.list(state: argResults["state"]);
    }
    if (argResults["DOWNLOAD"]) {
      if (argResults["state"] == "NotSet" || argResults["version"] == '0') {
        _log.severe(" state and version should be specify");
      } else {
        await _mgr.download(
            argResults["state"], int.parse(argResults["version"]));
      }
    }

    if (argResults["INFO"]) {
      if (argResults["state"] == "NotSet" || argResults["version"] == '0') {
        _log.severe(" state and version should be specify");
      } else {
        await _mgr.info(argResults["state"], int.parse(argResults["version"]));
      }
    }
    if (argResults["UPDATE"]) {
      if (argResults["state"] == "NotSet" ||
          argResults["version"] == '0' ||
          argResults["tag"] == "NotSet" ||
          argResults["vtag"] == '0') {
        _log.severe(" state and version, tag and vtag should be specify");
      } else {
        await _mgr.update(argResults["state"], int.parse(argResults["version"]),
            argResults["tag"], argResults["vtag"]);
      }
    }

    await _mgr.close();
  }

  if (argResults["RUN"]) {
    var _mgr =
        new mgr.mgRun("mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    // _mongoAccess = new mg.MongoAccess(
    //     "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mgr.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");

    if (argResults["LIST"]) await _mgr.list();

    if (argResults["INFO"]) {
      if (argResults["setup"] == "NotSet" || argResults["run"] == '0') {
        _log.severe(" setup and run should be specify");
      } else {
        await _mgr.info(argResults["setup"], int.parse(argResults["run"]));
      }
    }
    if (argResults["UPDATE"]) {
      if (argResults["setup"] == "NotSet" ||
          argResults["run"] == '0' ||
          argResults["tag"] == "NotSet" ||
          argResults["vtag"] == '0') {
        _log.severe(" run and setup, tag and vtag should be specify");
      } else {
        await _mgr.update(argResults["setup"], int.parse(argResults["run"]),
            argResults["tag"], argResults["vtag"]);
      }
    }

    await _mgr.close();
  }
  if (argResults["download-conf"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["config"] == "NotSet" || argResults["version"] == '0') {
      _log.severe(" config and version should be specify");
    } else {
      await _mongoAccess.downloadConfiguration(
          argResults["config"], int.parse(argResults["version"]));
    }
    await _mongoAccess.close();
  }
  if (argResults["upload-conf"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["file"] == "NotSet" || argResults["comment"] == 'NotSet') {
      _log.severe(" cfile and  comment should be specify");
    } else {
      await _mongoAccess.uploadConfiguration(
          argResults["file"], argResults["comment"]);
    }
    await _mongoAccess.close();
  }
  if (argResults["list-runs"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["setup"] == "NotSet") {
      await _mongoAccess.listRuns();
    } else {
      await _mongoAccess.listRuns(location: argResults["setup"]);
    }

    await _mongoAccess.close();
  }
  if (argResults["run-info"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["setup"] == "NotSet" || argResults["run"] == '0') {
      _log.severe(" run and setup should be specify");
    } else {
      await _mongoAccess.runInfo(
          argResults["setup"], int.parse(argResults["run"]));
    }

    await _mongoAccess.close();
  }
  if (argResults["update-run"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["setup"] == "NotSet" ||
        argResults["run"] == '0' ||
        argResults["tag"] == "NotSet" ||
        argResults["vtag"] == '0') {
      _log.severe(" run and setup, tag and vtag should be specify");
    } else {
      await _mongoAccess.updateRun(argResults["setup"],
          int.parse(argResults["run"]), argResults["tag"], argResults["vtag"]);
    }

    await _mongoAccess.close();
  }
  if (argResults["list-states"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");

    await _mongoAccess.listStates();

    await _mongoAccess.close();
  }
  if (argResults["download-state"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["state"] == "NotSet" || argResults["version"] == '0') {
      _log.severe(" state and version should be specify");
    } else {
      await _mongoAccess.downloadState(
          argResults["state"], int.parse(argResults["version"]));
    }
    await _mongoAccess.close();
  }
}

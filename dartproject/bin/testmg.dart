import 'mgaccess.dart' as mg;
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
    ..addOption('Configuration',
        abbr: 'c', defaultsTo: Platform.environment['DAQMONGO'])
    ..addOption('Command', abbr: 'M', defaultsTo: "NONE", help: "Command name")
    ..addOption('Application',
        abbr: 'a', defaultsTo: "BUILDER", help: "Application name")
    ..addOption('Parameters',
        abbr: 'p', defaultsTo: "{}", help: "Parameters list")
    ..addOption('run', defaultsTo: '0')
    ..addOption('setup', defaultsTo: 'UNKNOWN')
    ..addOption('vtag', defaultsTo: '0')
    ..addOption('tag', defaultsTo: 'UNKNOWN')
    ..addOption('comment', defaultsTo: 'Test run')
    ..addFlag('listruns',
        negatable: false, help: 'List all the run with optional setup')
    ..addFlag('runinfo',
        negatable: false, help: 'runinfo  with non-optional setup and run')
    ..addFlag('updaterun',
        negatable: false,
        help:
            'update runinfo  with non-optional setup and run and tag and vtag')
    ..addFlag('infos',
        abbr: 'i',
        negatable: false,
        help: "Displays all processes information.")
    ..addFlag('verbose', abbr: 'v', negatable: false, help: "Verbose mode")
    ..addFlag('jobcontrol',
        abbr: 'j', negatable: false, help: "jobcontrol flag")
    ..addFlag('daqcontrol',
        abbr: 'd', negatable: false, help: "daqcontrol flag")
    ..addFlag('create', abbr: 'R', negatable: false, help: "Create transition ")
    ..addFlag('initialise',
        abbr: 'I', negatable: false, help: "Initialise transition ")
    ..addFlag('configure',
        abbr: 'C', negatable: false, help: "Initialise transition ")
    ..addFlag('start', abbr: 'A', negatable: false, help: "Start transition ")
    ..addFlag('stop', abbr: 'O', negatable: false, help: "Stop transition ")
    ..addFlag('kill',
        abbr: 'K', negatable: false, help: "Kill transition of the job control")
    ..addFlag('destroy',
        abbr: 'D',
        negatable: false,
        help: "Destroy transition of the job control")
    ..addFlag('status', abbr: 's', negatable: false, help: "Status command ")
    ..addFlag('builder', negatable: false, help: "Event Builder flag ")
    ..addFlag('source', negatable: false, help: "Data Source flag ")
    ..addFlag('trigger', negatable: false, help: "trigger flag ")
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

  //print("job ${argResults['jobcontrol']} ${argResults['start']}");

  // print(argResults['server'] +
  //     " " +
  //     argResults['port'] +
  //     " " +
  //     argResults['mongo'] +
  //     " " +
  //     argResults['configuration']);

  // var rep = json.decode(await d.BuilderStatus());
  // print(rep);

  // rep = json.decode(await d.SourceStatus("EXSERVER"));
  // print(rep);

  // rep = json.decode(await d.TriggerStatus("TRIGGER"));
  // print(rep);

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

  _log.info(" Account is ${user} ${pwd} ${host} ${port} ${dbname}");
  if (argResults["listruns"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["setup"] == "UNKNOWN") {
      await _mongoAccess.listRuns();
    } else {
      await _mongoAccess.listRuns(location: argResults["setup"]);
    }

    await _mongoAccess.close();
  }
  if (argResults["runinfo"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["setup"] == "UNKNOWN" || argResults["run"] == '0') {
      _log.severe(" run and setup should be specify");
    } else {
      await _mongoAccess.runInfo(
          argResults["setup"], int.parse(argResults["run"]));
    }

    await _mongoAccess.close();
  }
  if (argResults["updaterun"]) {
    _mongoAccess = new mg.MongoAccess(
        "mongodb://${user}:${pwd}@${host}:${port}/${dbname}");
    await _mongoAccess.open();
    //await _mongoAccess.listRuns(location: "TRICOT2M2");
    if (argResults["setup"] == "UNKNOWN" ||
        argResults["run"] == '0' ||
        argResults["tag"] == "UNKNOWN" ||
        argResults["vtag"] == '0') {
      _log.severe(" run and setup, tag and vtag should be specify");
    } else {
      await _mongoAccess.updateRun(argResults["setup"],
          int.parse(argResults["run"]), argResults["tag"], argResults["vtag"]);
    }

    await _mongoAccess.close();
  }
}

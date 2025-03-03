#!/usr/bin/env python2.7
# encoding: utf-8

from __future__ import division
import sys
import os
import traceback
import numpy
import json
import logging
import re
import itertools
import glob
import operator
import csv

from argparse import ArgumentParser, RawDescriptionHelpFormatter
import tabulate

# hack to avoid installing of Aclib
import sys
import os
import inspect
cmd_folder = os.path.realpath(
    os.path.abspath(os.path.split(inspect.getfile(inspect.currentframe()))[0]))
cmd_folder = os.path.realpath(os.path.join(cmd_folder, ".."))
if cmd_folder not in sys.path:
    sys.path.insert(0, cmd_folder)

import matplotlib
matplotlib.use('Agg')

from plottingscripts.plotting.scatter import plot_scatter_plot

__version__ = 0.3
__date__ = '2014-09-22'
__updated__ = '2016-08-18'
__author__ = "Marius Lindauer"
__license__ = "BSD"


class RunGatherer(object):

    def __init__(self):

        self.logger = logging.getLogger("RunGatherer")

    def gather_runs(self, working_dir):
        pass

    def enumerate_folders(self, folder):
        for name in sorted(os.listdir(folder)):
            full_name = os.path.join(folder, name)
            if not (os.path.isdir(full_name) or os.path.islink(full_name)):
                continue
            yield name, full_name


class AClibRunGatherer(RunGatherer):

    def __init__(self):

        self.logger = logging.getLogger("AClibRunGatherer")

    def gather_runs(self, working_dir):
        for scenario, full_path_scen in self.enumerate_folders(working_dir):
            for configurator, full_path_conf in self.enumerate_folders(full_path_scen):
                if configurator not in ("SMAC2", "GGA", "PARAMILS", "ROAR"):
                    self.logger.warn("%s is not a configurator" % configurator)
                    #continue
                runs = (r[1] for r in self.enumerate_folders(full_path_conf))
                try:
                    yield Evaluator(runs, scenario, configurator)
                except IndexError:
                    self.logger.warn("Could not read %s" %(configurator))


class ACcloudRunGatherer(RunGatherer):

    def __init__(self):

        self.logger = logging.getLogger("ACcloudRunGatherer")

    def gather_runs(self, working_dir):
        try:
            fn = os.path.join(working_dir, 'runconfig.json')
            with open(fn, 'r') as f:
                config = json.load(f)
                scenario = config['experiment']['scenario']
                configurator = config['experiment']['configurator']
                seed = config['experiment'].get('seed', 1)
                n = config['experiment'].get('repetition', 1)
                if config['experiment'].get('consecutive_seed', False):
                    seeds = range(seed, seed + n)
                else:
                    seeds = [seed] * n
        except (KeyError, ValueError) as e:
            self.logger.error('Could not parse config file %s: ' % fn, e.msg)
            sys.exit(1)
        except IOError:
            self.logger.error('Could not open config file %s' % fn)
            sys.exit(1)
        results_dir = os.path.join(working_dir, 'results')
        runs = [r[1] for r in self.enumerate_folders(results_dir)]
        yield Evaluator(runs, scenario, configurator, seeds)


class CsscRunGatherer(RunGatherer):

    def __init__(self):

        self.logger = logging.getLogger("CsscRunGatherer")

    def gather_runs(self, working_dir):
        for folder, abs_folder in self.enumerate_folders(working_dir):
            scenario = folder
            try:
                configurator = (
                    conf for conf in ['smac', 'paramils'] if conf in folder).next()
            except StopIteration:
                self.logger.error(
                    'Could not determine configurator for folder %s' % abs_folder)
                continue
            logfilenames = glob.glob(os.path.join(abs_folder, 'log*.txt'))
            pattern = re.compile(r'.*log(\d+)\.txt')
            match_seed = lambda s: pattern.match(s)
            seeds = sorted([int(match.group(1))
                            for match in map(match_seed, logfilenames) if match])
            n = len(seeds)
            yield Evaluator(abs_folder * n, scenario, configurator, seeds)


class Evaluator(object):

    tabulate_fmt = 'pipe'

    def __init__(self, dirs, scenario, configurator, ids=None):

        self.logger = logging.getLogger("Evaluator")

        self.scenario = scenario
        self.configurator = configurator

        if not ids:
            ids = ((int(os.path.basename(d).split('-')[1]), d) for d in dirs)
        else:
            ids = zip(ids, dirs)

        self.single_runs = []
        for id, run_path in ids:
            self.single_runs.append(
                SingleRunEvaluator(run_path, scenario, configurator, id))

    def run(self):
        self.logger.info(
            'Reading %s@%sx%d' % (self.scenario, self.configurator, len(self.single_runs)))
        for single_run in self.single_runs:
            single_run.run()
        try:
            self.print_best_acrun()
        except KeyError:
            self.logger.debug('Could not determine best AC run')
            #traceback.print_exc()

        try:
            self.print_further_stats()
        except KeyError:
            self.logger.debug(
                "Could not get test performances of incumbent configurations")
            #traceback.print_exc()

        self.print_stats()

    def print_best_acrun(self):
        par10_inc_trains = [(id, x.stats["inc_train"]["PAR10"])
                            for id, x in enumerate(self.single_runs)]
        sorted_par10 = sorted(par10_inc_trains, key=lambda x: x[1])
        best_run = sorted_par10[0][0]
        self.logger.info("Best AC Run (%d):" % (best_run))
        self.logger.info("Run %d [Cost on Train]: %.4f (Def) vs. %.4f (Opt)" % (best_run,
                                                                                self.single_runs[best_run].stats.get(
                                                                                    "def_train", {}).get("PAR10", -256),
                                                                                self.single_runs[best_run].stats[
                                                                                    "inc_train"]["PAR10"],
                                                                                ))

        self.logger.info("Run %d [Cost on Test]: %.4f (Def) vs. %.4f (Opt)" % (best_run,
                                                                               self.single_runs[best_run].stats.get(
                                                                                   "def_test", {}).get("PAR10", -256),
                                                                               self.single_runs[best_run].stats[
                                                                                   "inc_test"]["PAR10"],
                                                                               ))

    def print_further_stats(self):
        median_par10 = numpy.median(
            [x.stats["inc_test"]["PAR10"] for x in self.single_runs])
        self.logger.info("Median Cost: %.3f" % (median_par10))
        mean_par10 = numpy.mean(
            [x.stats["inc_test"]["PAR10"] for x in self.single_runs])
        self.logger.info("Mean Cost: %.3f" % (mean_par10))
        std_par10 = numpy.std(
            [x.stats["inc_test"]["PAR10"] for x in self.single_runs])
        self.logger.info("Std Cost: %.3f" % (std_par10))

    def print_stats(self):
        try:
            data = (
                   (run.stats['runs'],
                    '{:.2%}'.format(
                        run.stats['cencored_runs'] / run.stats['runs']),
                    run.stats['iterations'],
                    run.stats['instances'],
                    run.stats['configurations'],
                    run.stats['avg_response'],
                    run.stats['avg_runtime'],
                    run.stats['avg_quality']
                    ) for run in self.single_runs)
            self.logger.info(
            'SMAC stats for %s@%s' % (self.scenario, self.configurator))
            print(tabulate.tabulate(data, headers=[
                                    '#runs',
                                    '% cencored',
                                    '#SMAC_iterations',
                                    '#instances',
                                    '#configurations',
                                    'avg_response',
                                    'avg_runtime',
                                    'avg_quality',
                                    ], tablefmt=self.tabulate_fmt))
        except KeyError:
            self.logger.debug(
                "Have not found runs_and_results file -- cannot print configuration statistics")


class SingleRunEvaluator(object):
    test_inc_dir = 'validate-inc-test'
    test_def_dir = 'validate-def-test'
    train_inc_dir = "validate-inc-train"
    train_def_dir = "validate-def-train"
    obj_matrix_inc_fn = 'validationObjectiveMatrix*-traj*-walltime*.csv'
    obj_matrix_def_fn = 'validationObjectiveMatrix*-cli*-walltime*.csv'
    overtime_test_dir = "validate-time-test"
    overtime_train_dir = "validate-time-train"
    smac_dir = 'smac-output/aclib'

    def __init__(self, dir_, scenario, configurator, id_=1):
        '''
            Constructor
        '''
        self.logger = logging.getLogger("SingleRunEvaluator")

        self.dir_ = dir_
        self.scenario = scenario
        self.configurator = configurator
        self.serial = id_

        self.cutoff = None
        self.metric = "runtime"
        self.val_files = {}
        self.conf_files = {}
        self.stats = {}  # ["INC"|"DEF"]->["PAR1"|"PAR10"|"TOs"|"PI"]

    def run(self):
        '''
            1. reading data from <working-dir>/<sceanario>/<configurator>/run-<ID>/.../*.csv generated by smac-validate
            2. read cutoff from ...
            3. printing summary statistics
            4. scatter plots
        '''
        self.val_files = self.find_val_logs()

        if self.cutoff is None:
            self.logger.error(
                "Cutoff time was not defined in scenario file (%s)" % (self.dir_))
            sys.exit(1)

        for val_type, log_file in self.val_files.items():
            if "def" in val_type:
                par1, par10, tos, inst_perf_dict = self._read_val_logs(
                    log_file, self.cutoff, def_=True)
            else:
                par1, par10, tos, inst_perf_dict = self._read_val_logs(
                    log_file, self.cutoff, def_=False)
            self.stats[val_type] = dict(
                PAR1=par1, PAR10=par10, TOS=tos, PI=inst_perf_dict)

        self.conf_files = self.find_conf_files()
        for conf_type, log_file in self.conf_files.items():
            getattr(self, '_parse_{}'.format(conf_type))(log_file)

        if 'def_test' in self.stats and plot_scatter:
            self.logger.info("Plot scatter plot")
            self.plot_def_vs_inc()

        if 'inc_train' in self.stats and 'def_train' in self.stats:
            self.print_par10_train()

    def _parse_runs_and_results(self, filename, delimiter=','):
        header = True
        with open(filename, 'r') as f:
            instances = set()
            configurations = set()
            configurations = set()
            aggregated_response = 0
            aggregated_runtime = 0
            aggregated_quality = 0
            cencored_runs = 0
            run_number = 0
            data = csv.reader(f)
            for row in data:
                if header:
                    header = False
                    continue
                configurations.add(int(row[1]))
                instances.add(int(row[2]))
                cencored = row[4] == '1'
                if not cencored:
                    aggregated_response += float(row[3])
                    aggregated_runtime += float(row[7])
                    aggregated_quality += float(row[10])
                else:
                    cencored_runs += 1
            else:  # Last row
                self.stats['runs'] = int(row[0])
                self.stats['iterations'] = int(row[11])
            self.stats['instances'] = len(instances)
            self.stats['configurations'] = len(configurations)
            n = self.stats['runs']
            self.stats['avg_response'] = aggregated_response / n
            self.stats['avg_runtime'] = aggregated_runtime / n
            self.stats['avg_quality'] = aggregated_quality / n
            self.stats['cencored_runs'] = cencored_runs

    def find_conf_files(self):
        files = {}
        try:
            files['runs_and_results'] = glob.glob(os.path.join(
                self.dir_, SingleRunEvaluator.smac_dir, 'state-run%d' % self.serial, 'runs_and_results-it*.csv'))[0]
        except IndexError:
            self.logger.debug('Did not find runs and results')
            # sys.exit(0)
        return files

    def find_val_logs(self):
        '''
            read logs from <working-dir>/<sceanario>/<configurator>/run-<ID>/.../*.csv generated by smac-validate
        '''

        files = {}

        # READ CUTOFF from scenario file in state-run
        try:
            scenario_guess = os.path.join(self.dir_, "scenarios", "*", self.scenario, "scenario.txt")
            scenario_fn = glob.glob(scenario_guess)[0]
        except IndexError:
            self.logger.warn(
                "Didn't find scenario file: %s" %(scenario_guess))
            sys.exit(1)

        self.cutoff, self.metric = self._read_scenario(scenario_fn)

        try:
            files['inc_test'] = glob.glob(os.path.join(
                self.dir_, self.test_inc_dir, SingleRunEvaluator.obj_matrix_inc_fn))[0]
        except IndexError:
            self.logger.debug(
                'No performance data of incumbent on testing (-- will try later to look for \"validation over time\" results')

        try:
            files['def_test'] = glob.glob(os.path.join(
                self.dir_, SingleRunEvaluator.test_def_dir, SingleRunEvaluator.obj_matrix_def_fn))[0]
        except IndexError:
            self.logger.debug(
                'No performance data of default on testing found in {}'.format(self.dir_))

        try:
            files['inc_train'] = glob.glob(os.path.join(
                self.dir_, SingleRunEvaluator.train_inc_dir, SingleRunEvaluator.obj_matrix_inc_fn))[0]
        except IndexError:
            self.logger.debug(
                "No performance data of incumbent on training found")

        try:
            files['def_train'] = glob.glob(os.path.join(
                self.dir_, SingleRunEvaluator.train_def_dir, SingleRunEvaluator.obj_matrix_def_fn))[0]
        except IndexError:
            self.logger.debug(
                "No performance data of default on training found")

        if not files.get('inc_test'):
            try:
                files['inc_test'] = glob.glob(os.path.join(
                    self.dir_, self.overtime_test_dir, SingleRunEvaluator.obj_matrix_inc_fn))[0]
                files['def_test'] = glob.glob(os.path.join(
                    self.dir_, self.overtime_test_dir, SingleRunEvaluator.obj_matrix_inc_fn))[0]
                self.logger.debug(
                    "Reading \"validation of over time\" as validation test results")
            except IndexError:
                self.logger.debug(
                    'No performance data of incumbent on testing at \"test validation over time\"')

        if not files.get('inc_train'):
            try:
                files['inc_train'] = glob.glob(os.path.join(
                    self.dir_, self.overtime_train_dir, SingleRunEvaluator.obj_matrix_inc_fn))[0]
                files['def_train'] = glob.glob(os.path.join(
                    self.dir_, self.overtime_train_dir, SingleRunEvaluator.obj_matrix_inc_fn))[0]
                self.logger.debug(
                    "Reading \"validation of over time\" as validation train results")
            except IndexError:
                self.logger.debug(
                    'No performance data of incumbent on testing at \"train validation over time\"')

        return files

    def plot_def_vs_inc(self):
        x_data, y_data = self.__extract_values_in_order(
            self.stats["def_test"]["PI"], self.stats["inc_test"]["PI"])

        matplotlib.pyplot.close()
        fig = plot_scatter_plot(numpy.array(x_data), numpy.array(y_data), 
                          labels=["Default", "Configured"], 
                          max_val=self.cutoff,
                          linefactors=[2, 10, 100], 
                          metric=self.metric)
        fig.savefig(os.path.join(".", "scatter_%s_%s_%d.pdf" % (self.scenario, self.configurator, self.serial)))
        self.logger.info("Run %d [Cost on Test]: %.4f (Def) vs. %.4f (Opt)" % (self.serial,
                                                                               self.stats["def_test"][
                                                                                   "PAR10"],
                                                                               self.stats["inc_test"][
                                                                                   "PAR10"],
                                                                               ))

    def print_par10_train(self):
        self.logger.info("Run %d [Cost on Train]: %.4f (Def) vs. %.4f (Opt)" % (self.serial,
                                                                                self.stats["def_train"][
                                                                                    "PAR10"],
                                                                                self.stats["inc_train"][
                                                                                    "PAR10"],
                                                                                ))

    def _read_val_logs(self, csv_file, cutoff, def_=False):
        '''
            read csv file generated by smac-validate
            (Format: "Configuration","Seed","Instance","Response")
            and returns PAR1, PAR10, #TOs and dict: inst_name -> performance
        '''
        par1 = 0.0
        par10 = 0.0
        tos = 0
        insts = 0
        inst_perf_dict = {}
        runtime_obj = self.metric == "runtime"
        with open(csv_file) as fp:
            for line in fp:
                #_,_,instance,response = line.split(",")
                splits = line.split(",")
                instance = splits[0]
                _seed = splits[1]
                if def_:
                    response = splits[2]
                else:
                    response = splits[-1]  # take only the last one
                try:
                    runtime = float(response.strip("\n").strip("\""))
                except:
                    continue
                instance = instance.strip("\"")
                inst_perf_dict[instance] = runtime
                par1 += runtime
                if runtime_obj and runtime >= cutoff:
                    runtime *= 10
                    tos += 1
                par10 += runtime
                insts += 1
        # <gothm> Made this explicit float division by importing future
        return par1 / insts, par10 / insts, tos, inst_perf_dict

    def _read_scenario(self, scenario_fn):
        ''' read cutoff_time from scenario file '''
        cutoff = None
        metric = "runtime"
        with open(scenario_fn) as fp:
            for line in fp:
                line = line.replace("\n","").strip(" ")
                if line.startswith("cutoff_time") or \
                        line.startswith("cutoff-time") or \
                        line.startswith("algo-cutoff-time") or \
                        line.startswith("cutoffTime"):
                    cutoff = float(line.split("=")[1])
                if line.startswith("run_obj"):
                    metric = line.split("=")[1].strip(" ")
        return cutoff, metric

    def __extract_values_in_order(self, dic1, dic2):
        '''
            extract the values of two dictionaries in the same order
            Assumption: both have the same keys
        '''
        vec1 = []
        vec2 = []
        for k in dic1.keys():
            vec1.append(dic1[k])
            vec2.append(dic2[k])
        return vec1, vec2


class CompareConfigurators(object):

    def __init__(self, table_format):
        ''' Constructor'''
        self.table_format = table_format
        self.logger = logging.getLogger("CompareConfigurators")

    def compare(self, scen_2_runs):
        self.get_table_of_medians(scen_2_runs)
        self.get_table_of_bests(scen_2_runs)

    def get_table_of_medians(self, scen_2_runs):
        self.logger.info("Table of median performances:")
        data = {}
        configurators = set()
        
        for scen, runs in scen_2_runs.items():
            row = [scen]
            for r in runs:
                try:
                    median_par10 = numpy.median(
                        [x.stats["inc_test"]["PAR10"] for x in r.single_runs])
                    data[scen] = data.get(scen,{})
                    data[scen][r.configurator] = "%.2f" %(median_par10)
                    configurators.add(r.configurator)
                except KeyError:
                    self.logger.warn("Could not get inc_test for %s on %s" %(str(r.configurator), scen))

        configurators = sorted(list(configurators))
        renamed_configurators = [name_mapping.get(c,c) for c in configurators] 
        data_tab = [["Scenario"] + renamed_configurators]
        for scen, d in data.items():
            data_tab.append([scen] + [str(d.get(configurator,"NaN")) for configurator in configurators])
        
        print(tabulate.tabulate(data_tab, tablefmt=self.table_format))
        
    def get_table_of_bests(self, scen_2_runs):
        self.logger.info("Table of best runs (selected on train and validated on test):")
        data = {}
        configurators = set(["Default"])
        
        for scen, runs in scen_2_runs.items():
            row = [scen]
            for r in runs:
                try:
                    best_run = numpy.argmin(
                        [x.stats["inc_train"]["PAR10"] for x in r.single_runs])
                    data[scen] = data.get(scen,{})
                    data[scen][r.configurator] = "%.2f" %(r.single_runs[best_run].stats["inc_test"]["PAR10"])
                    data[scen]["Default"] = "%.2f" %(r.single_runs[best_run].stats["def_test"]["PAR10"])
                    configurators.add(r.configurator)
                except KeyError:
                    #self.logger.warn("Could not get inc_test for %s on %s" %(str(r.configurator), scen))
                    pass

        configurators = sorted(list(configurators))
        renamed_configurators = [name_mapping.get(c,c) for c in configurators] 
        data_tab = [["Scenario"] + renamed_configurators]
        for scen, d in data.items():
            data_tab.append([scen] + [str(d.get(configurator,"NaN")) for configurator in configurators])
        
        print(tabulate.tabulate(data_tab, tablefmt=self.table_format))


def main(argv=None):  # IGNORE:C0111
    '''Command line options.'''

    if argv is None:
        argv = sys.argv
    else:
        sys.argv.extend(argv)

    #program_name = os.path.basename(sys.argv[0])
    program_version = "v%s" % __version__
    program_build_date = str(__updated__)
    program_version_message = "%%(prog)s %s (%s)" % (
        program_version, program_build_date)
    program_license = '''

  Created by Marius Lindauer on %s.
  Copyright 2016 - AClib. All rights reserved.
  
  Licensed under the BSD
  
  Distributed on an "AS IS" basis without warranties
  or conditions of any kind, either express or implied.

USAGE
''' % (str(__date__))

    aclib_root = os.path.split(os.path.split(__file__)[0])[0]
    gatherers = {
        "AClib": AClibRunGatherer,
        "ACcloud": ACcloudRunGatherer,
        "CSSC": CsscRunGatherer
    }
    try:
        parser = ArgumentParser(
            description=program_license, formatter_class=RawDescriptionHelpFormatter)
        parser.add_argument("--folder_structure", dest="gatherer", choices=gatherers,
                            default="AClib", help="The folder structure standard in which the runs are organized")
        parser.add_argument("--table_format", dest="table_format", choices=tabulate.tabulate_formats,
                            default="pipe", help="Output format of generated tables")
        parser.add_argument("--name_map_json", default=None, 
                        help="A json file with a mapping from AC names to names"
                             " to be used in plots")
        parser.add_argument("--scatter_plots", default=False,
                            action="store_true", 
                            help="generates *all* scatter plots")
        parser.add_argument("--verbose", default="INFO",
                            choices = ["INFO", "DEBUG"], 
                            help="verbosity level")
        
        args = parser.parse_args()
        
        root_logger = logging.getLogger()
        root_logger.setLevel(args.verbose)
        logger_handler = logging.StreamHandler(
            stream=sys.stdout)
        root_logger.addHandler(logger_handler)
        
        Evaluator.tabulate_fmt = args.table_format

        global name_mapping
        if args.name_map_json:
            with open(args.name_map_json) as fp:
                name_mapping = json.load(fp)
        global plot_scatter
        plot_scatter = args.scatter_plots

        scen_2_runs = {}
        rg = gatherers[args.gatherer]()
        runs = rg.gather_runs(".")
        for run in runs:
            try:
                run.run()
                scen_2_runs[run.scenario] = scen_2_runs.get(run.scenario, [])
                scen_2_runs[run.scenario].append(run)
            except IndexError:
                logging.warn("Could not read directory .. go on")
                # raise

        cc = CompareConfigurators(table_format=args.table_format)
        cc.compare(scen_2_runs=scen_2_runs)

    except (KeyboardInterrupt, SystemExit):
        ### handle keyboard interrupt ###
        return 1

if __name__ == "__main__":
    name_mapping = {}
    plot_scatter = False
    sys.exit(main())

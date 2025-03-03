import os
import shutil

def rm_features(scenario_fn:str, exp_dir:str,
                      out_fn:str="scenario.txt"):
    '''
        removes "feature_file" from scenario
        
        Arguments
        ---------
        scenario_fn : str
            scenario file name
        exp_dir: str
            experiment directory
        out_fn:str
            file name of new scenario file in exp_dir
            
        Returns
        -------
        out_fn: str
    '''
    
    new_scen_fn = os.path.join(exp_dir, out_fn)
    shutil.copy(os.path.join(exp_dir, scenario_fn), os.path.join(exp_dir, "scenario.back"))
    with open(new_scen_fn, "w") as fp_new:
        with open(os.path.join(exp_dir, "scenario.back")) as fp:
            for line in fp:
                if "feature_file" in line:
                    continue
                else:
                    fp_new.write(line)
    return out_fn

def enum_features(scenario_fn:str, exp_dir:str,
                      out_fn:str="scenario.txt"):
    '''
        replaces instance features with the id of each instance (1,2,3,...)
        
        Arguments
        ---------
        scenario_fn : str
            scenario file name
        exp_dir: str
            experiment directory
        out_fn:str
            file name of new scenario file in exp_dir
            
        Returns
        -------
        out_fn: str
    '''
    
    new_scen_fn = os.path.join(exp_dir, out_fn)
    new_feat_fn = os.path.join(exp_dir, "enum_feats.txt")
    shutil.copy(os.path.join(exp_dir, scenario_fn), os.path.join(exp_dir, "scenario.back"))
    with open(new_scen_fn, "w") as fp_new:
        with open(os.path.join(exp_dir, "scenario.back")) as fp:
            for line in fp:
                if "feature_file" in line:
                    feat_fn = line.split("=")[1].strip()
                    fp_new.write("feature_file = enum_feats.txt\n")
                    _id_features(feature_fn=os.path.join(exp_dir,feat_fn), 
                                 out_fn=os.path.join(exp_dir,"enum_feats.txt"))
                else:
                    fp_new.write(line)
    return out_fn

def _id_features(feature_fn:str, out_fn:str):
    '''
        replaces instance features with the id of each instance (1,2,3,...)
        
        Arguments
        ---------
        feature_fn : str
            originial feature file
        out_fn: str
            new feature file
    '''
    
    with open(feature_fn) as fp:
        with open(out_fn, "w") as out_fp:
            out_fp.write("instance,ID\n")
            fp.readline() # skip header
            for idx, line in enumerate(fp):
                try:
                    inst = line.split(",")[0]
                    out_fp.write("%s,%d\n" %(inst,idx))
                except IndexError:
                    pass
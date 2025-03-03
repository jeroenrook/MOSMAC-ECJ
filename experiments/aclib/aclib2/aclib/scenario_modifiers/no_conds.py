import os
import shutil

def no_conds_scenario(scenario_fn:str, exp_dir:str,
                      out_fn:str="scenario.txt"):
    '''
        writes new pcs without conditional constraints
        and updates scenario file correspondingly in exp_dir
        
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
    new_pcs_fn = os.path.join(exp_dir,"params_no_conds.txt")
    shutil.copy(os.path.join(exp_dir, scenario_fn), os.path.join(exp_dir, "scenario.back"))
    with open(new_scen_fn, "w") as fp_new:
        with open(os.path.join(exp_dir, "scenario.back")) as fp:
            for line in fp:
                if "paramfile" in line:
                    old_pcs = line.replace("\n","").split("=")[1].strip()
                    no_conds_pcs(pcs_fn=os.path.join(exp_dir, old_pcs), 
                                 fn_out=new_pcs_fn)
                    fp_new.write("paramfile = params_no_conds.txt\n")
                else:
                    fp_new.write(line)
    return out_fn

def no_conds_pcs(pcs_fn:str, fn_out:str=None):
    '''
        removes all conditional constraints from pcs file
        and prints new pcs in case of fn_out==None or 
        writes new pcs into fn_out
        
        Arguments
        ---------
        pcs_fn: str
            file name of input pcs file
        granularity: int
            number of values of each discretized parameter (maybe plus default value)
        fn_out: str
            file name of output file
    '''
    
    if fn_out is None:
        fp_out = None
    else:
        fp_out = open(fn_out, "w")
        
    with open(pcs_fn, "r") as fp:
        for line in fp:
            line = line.strip("\n")
            if line.find("#") > -1:
                line = line[:line.find("#")]  # remove comments
            if line == "":
                continue
            if "|" not in line:
                if fp_out is None:
                    print(line)
                else:
                    fp_out.write(line+"\n")

    if fp_out is not None:
        fp_out.flush()
        fp_out.close()
    
    
    
    
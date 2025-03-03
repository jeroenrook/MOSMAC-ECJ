from functools import partial
import logging

import numpy as np
import scipy.stats


def test_per_timestep(performances, alpha=0.05, compare_to_indx=None):
    
    steps = len(performances[0][0])
    
    p_values_all = []
    medians_all = []
    for s in range(steps):
        medians = [np.median(ac[:,s]) for ac in performances] 
        medians_all.append(medians)
        if not compare_to_indx is None:
            best_ac = compare_to_indx
        else:
            best_ac = np.argmin(medians)
        x = performances[best_ac][:,s]
        p_values = []
        for ac_data in performances:
            if len(ac_data[:,0]) >= 20:
                test_func = partial(scipy.stats.mannwhitneyu, alternative='greater')
            else:
                test_func = scipy.stats.ranksums
            y = ac_data[:,s]
            if np.all(x==y):
                p_value = 1 
            else:
                _,p_value = test_func(x,y)
            p_values.append(p_value)
        p_values_all.append(p_values)

    return np.array(p_values_all).T, np.array(medians_all).T


def speedups_to_ref(performances,compare_to_indx,time_lists, alpha=0.05):
    #final_ref = np.median(performances[compare_to_indx],axis=0)[-1]
    final_ref = performances[compare_to_indx][:,-1]
    init_ref = performances[compare_to_indx][:,0]
    #test_func = partial(scipy.stats.mannwhitneyu, alternative='greater')
    final_t = time_lists[compare_to_indx][-1]
    speedups = []
    #print(final_t)
    for time_list, ac in zip(time_lists, performances):
        speedup = 1
        logging.debug(">>>>>>>>>>>>>>>")
        for s,t in enumerate(time_list):
            if t > final_t:
                break
            x_ = ac[:,s]
            try:
                p_value_final = perm_unpaired_test(final_ref,x_)
                p_value_init = perm_unpaired_test(x_,init_ref)
                #_,p_value_final = test_func(x_, final_ref)
                #_,p_value_init = test_func(init_ref, x_)
                print(t,p_value_init,p_value_final)
            except ValueError:
                p_value_final, p_value_init = 1, 1
            if p_value_final > alpha and p_value_init < alpha:
                speedup = max(speedup,final_t / t)
            else: # should not get worse again
                speedup = 1
            #print(speedup)
        speedups.append(speedup)
    speedups = np.array(speedups, dtype=np.float)
    speedups /= speedups[compare_to_indx] # normalize by reference
    return np.array(speedups)


def bootstrapping_speedups_to_ref(performances,compare_to_indx,time_lists, n_boostrap_samples=10000):
    final_ref = performances[compare_to_indx][:,-1]
    final_t = time_lists[compare_to_indx][-1]
    n_ref_runs = performances[compare_to_indx].shape[0]
    
    speedups = []
    for time_list, ac in zip(time_lists, performances):
        sampled_speedups = []
        n_ac_runs = ac.shape[0]
        for _ in range(n_boostrap_samples):
            sample_idrun_ref = np.random.randint(0,n_ref_runs) 
            sample_run_ref = performances[compare_to_indx][sample_idrun_ref,:]
            sample_idrun_ac = np.random.randint(0,n_ac_runs)
            sample_run_ac = ac[sample_idrun_ac,:]
            
            # reference AC should not be just by over-tuning
            ref_value = min(sample_run_ref[0], sample_run_ref[-1])
            sample_value = min(sample_run_ac[0],sample_run_ac[-1])
            
            speedup = 1
            if sample_run_ref[0] <=  sample_run_ac[-1]: # final not better than beginning
                speedup = 1
            elif ref_value > sample_value: # ac better
                speedup = __speedup_two_indv_runs(xs=sample_run_ac, 
                                                  times=time_list, 
                                                  p=ref_value, 
                                                  t_p=final_t)
            elif ref_value < sample_value: # ref better
                speedup = 1 / __speedup_two_indv_runs(xs=sample_run_ref, 
                                                  times=time_lists[compare_to_indx], 
                                                  p=sample_value, 
                                                  t_p=time_list[-1])
            else:
                speedup = 1
                
            sampled_speedups.append(speedup)
        speedups.append(scipy.stats.mstats.gmean(sampled_speedups))
    
    return np.array(speedups)


def __speedup_two_indv_runs(xs,times,p,t_p):
    ''' speedup of x over p on time series t'''
    speedup = t_p / times[-1]
    for x,t in reversed(list(zip(xs,times))):
        if x < p:
            speedup = t_p/t
        else:
            return speedup
        
    return speedup


def get_auc(performances):        
    return np.array([[np.mean(np.median(X,axis=0)) for X in performances]])


def perm_unpaired_test(x, y, reps:int=10000):
    '''
        one-sided unpaired permutation test
        Alternative Hypothese: x is sig better than y
        
        Arguments
        ---------
        x: np.ndarray
        y: np.ndarray
        reps: int
            number of samples/repetitions
            
        Returns
        -------
        p-value
        
    '''
    x = np.array(x)
    y = np.array(y)
    if np.all(x == y):
        return 1.0

    if x.shape[0] != y.shape[0]:
        return perm_unpaired_unbalanced_test(x=x, y=y, reps=reps)
    
    ground_truth = np.sum(x) - np.sum(y)
    all_samples = np.concatenate((x,y))
    n_half = int(len(all_samples)/2)
    stats = []
    for _ in range(reps):
        np.random.shuffle(all_samples) # inplace
        stats.append(np.sum(all_samples[:n_half]) - np.sum(all_samples[n_half:]))
    p = scipy.stats.percentileofscore(a=stats, score=ground_truth) / 100
    return p


def perm_unpaired_unbalanced_test(x, y, reps: int = 10000):
    '''
        one-sided unpaired permutation test
        Alternative Hypothese: x is sig better than y
        Special implementation where x.shape != y.shape
        --> subsample larger set in each iteration

        Arguments
        ---------
        x: np.ndarray
        y: np.ndarray
        reps: int
            number of samples/repetitions

        Returns
        -------
        p-value

    '''
    ground_truth = np.average(x) - np.average(y)
    small_n = np.min((x.shape[0], y.shape[0]))
    smaller_y = x.shape[0] > y.shape[0]
    stats = []
    for _ in range(reps):
        if smaller_y:
            np.random.shuffle(x)
            x_ = x[:small_n]
            y_ = y
        else:
            np.random.shuffle(y)
            y_ = y[:small_n]
            x_ = x
        all_samples = np.concatenate((x_,y_))
        np.random.shuffle(all_samples)  # inplace
        stats.append(np.average(all_samples[:small_n]) - np.average(all_samples[small_n:]))
    p = scipy.stats.percentileofscore(a=stats, score=ground_truth) / 100
    return p


def perm_paired_test(x, y, reps:int=10000):
    '''
        one-sided paired permutation test
        Alternative Hypothese: x is sig better than y
        
        Arguments
        ---------
        x: np.ndarray
        y: np.ndarray
        reps: int
            number of samples/repetitions
            
        Returns
        -------
        p-value
        
    '''
    x = np.array(x)
    y = np.array(y)
    if np.all(x == y):
        return 1
    if x.shape[0] != y.shape[0]:
        raise ValueError("x and y have to have the same size for paired permutation test")

    ground_truth = np.sum(x-y)
    permutations = [np.sum((x-y) * np.random.choice([1,-1], size=x.shape[0])) for _ in range(reps)]
    p = scipy.stats.percentileofscore(a=permutations, score=ground_truth) / 100
    
    #print(p)
    return p

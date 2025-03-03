import sys
import os
import time

os.environ["OPENBLAS_NUM_THREADS"] = "1"

# from sklearnex import patch_sklearn
# patch_sklearn()

import random
import math
import datetime
import numpy as np
from sklearn.svm import SVC
from sklearn.model_selection import KFold
from sklearn.metrics import precision_score, recall_score, accuracy_score, f1_score


TRAIN_FRAC = 0.9

inst = sys.argv[1]
fn = sys.argv[2]
seed = max(0, int(sys.argv[3]))
config = sys.argv[4:]

if not(inst.startswith("i") or inst.startswith("test")):
    raise ValueError("instance has to be i%d or test")

rng = np.random.RandomState(12345)

# read in data
print("Load from %s " % (fn))

data = np.loadtxt(fn, delimiter=",", dtype=float)

rng.shuffle(data)
y = np.array(data[:, -1], dtype=int)
X = np.array(data[:, :-1], dtype=float)

del data

n_train = math.ceil(X.shape[0] * TRAIN_FRAC)

X_train = X[:n_train, :]
y_train = y[:n_train]

X_test = X[n_train:, :]
y_test = y[n_train:]

# if instance starts with "i", use i-th cv-split on trainings data
if inst.startswith("i"):
    fold_indx = int(inst[1:]) - 1
    print("Use %dth-cv split" %(fold_indx))
    kfold = KFold(n_splits=10, shuffle=False)
    kf = list(kfold.split(X=X_train))
    train_index, test_index = kf[fold_indx]
    X_train_fold = X_train[train_index, :]
    y_train_fold = y_train[train_index]
    X_test_fold = X_train[test_index, :]
    y_test_fold = y_train[test_index]
    X_train, y_train = X_train_fold, y_train_fold
    X_train, y_train = X_train_fold[:5000, :], y_train_fold[:5000] # TODO for faster iterations during development
    X_test, y_test = X_test_fold, y_test_fold
    
# set params of xgboost
param = {"random_state": seed, "cache_size": 1000}

iter_conf = iter(config)
int_params = ["degree", "cache_size", "max_iter"]
str_params = ["kernel", "gamma"]
bool_params = ["shrinking"]
while True:
    try:
        name = next(iter_conf).strip("-")
        value = next(iter_conf)
        if name in int_params:
            param[name] = int(value)
        elif name in str_params:
            param[name] = str(value)
        elif name in bool_params:
            if value == "True":
                param[name] = True
            else:
                param[name] = False
        else:
            param[name] = float(value)
    except StopIteration:
        break


# Make sure inactive params are not present
if param["gamma"] == "float":
    param["gamma"] = param["gamma_val"]
    try:
        del param["gamma_val"]
    except:
        pass
if param['kernel'] == "rbf":
    try:
        del param["degree"]
    except:
        pass
    try:
        del param["coef0"]
    except:
        pass

if param["kernel"] == "sigmoid":
    try:
        del param["degree"]
    except:
        pass

print(param)

print("Train on %d points" %(y_train.shape[0]))
svc = SVC(**param)
start_time = time.perf_counter()
svc.fit(X_train, y_train)
print(f"fittime {time.perf_counter()-start_time}")
preds = svc.predict(X_test)

acc = float(np.sum(preds == y_test)) / y_test.shape[0]
print("Error: %.6f" %(1-acc))

print(f"{accuracy_score.__name__} {1 - accuracy_score(y_test, preds):.6f}")
for score in [precision_score, recall_score, f1_score]:
    print(f"{score.__name__} {1 - score(y_test, preds, average='macro'):.6f}")

print(f"cache_size {svc.cache_size}")

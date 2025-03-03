import sys

from ConfigSpace.read_and_write.pcs import read

pcs_fn = sys.argv[1]
with open(pcs_fn, 'r') as fp:
    cs = read(fp)

def_conf = cs.get_default_configuration()

names = []
values = []
for p in sorted(cs.get_hyperparameter_names()):
    p_ = p.replace("@","x").replace(":","_").replace("-","_")
    names.append(p_)
    values.append('NA' if def_conf[p] is None else str(def_conf[p]))

print(" ".join(names))
print(" ".join(values))




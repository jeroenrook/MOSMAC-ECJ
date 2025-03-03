from pathlib import Path
from ..base import ExpScenario


class MLExpScenario(ExpScenario):
    def so_procedure(self, output_dir: Path = Path("./output/"), seed=1):
        n_obj = len(self.objectives)
        if self.runcount_limit is not None:
            self.runcount_limit = int(self.runcount_limit * ((1 + n_obj) / n_obj))
        if self.wallclock_limit is not None:
            self.wallclock_limit = int(self.wallclock_limit * ((1 + n_obj) / n_obj))
        self.objectives.append({"name": "f1", "cost_for_crash": float(2**32)})

        return super().so_procedure(output_dir, seed)
library("optparse")
library("jsonlite")
library("smoof")
library("ecr3vis")
# library("tidyverse")


get_options <- function() {
  option_list <- list(
    # Input options
    optparse::make_option("--instance", type = "character", default = NULL, help = "Instance filepath"),
    optparse::make_option("--dimensions", type = "numeric", default = 2, help = "Dimensions of the decision space"),
    optparse::make_option("--budget", type = "numeric", default = 10000L, help = "The maximum number of allowed function evaluations"),
    optparse::make_option("--seed", type = "numeric", default = 0, help = "The random seed"),
    # Output options
    optparse::make_option("--output", type = "character", default = NULL, help = "File to write the measured objective values to"),
    optparse::make_option("--save_solution", type = "character", default = NULL, "save solution set to an Rdata object"),
    optparse::make_option("--visualise", type = "character", default = NULL, help = "visualise population and solution set to a pdf")
  )
  return(option_list)
}

parse_instance_file <- function(filename, dimensions = 2) {
  content <- readChar(filename, nchars = file.info(filename)$size)
  # Change dimensions if possible
  if (dimensions != 2){
    if (!stringr::str_detect(content, "dimensions = 2")){
      stop("The function does not support other dimensions than 2")
    }
    content <- stringr::str_replace(content, "dimensions = 2", paste("dimensions =", dimensions))
  }
  obj.fn <- eval(parse(text = content))
  obj.fn <- smoof::addCountingWrapper(obj.fn)
  obj.fn <- smoof::addLoggingWrapper(obj.fn, logg.x = TRUE, logg.y = TRUE, size = 10000)
  return(obj.fn)
}

compute_performance_metrics <- function(population,
                                        fn,
                                        instance_path) {

    # Get reference data: Reference point + Reference HV

    ### COMPUTE METRICS ###
    # Population
        # HV-POP
        # HV-POP-NORM
        # SP-POP
    # ND Population
        # SP-ND-POP
    # ND Archive
        # HV-ARC
        # HV-ARC-NORM
        # SP-ND-ARC

    measures <- list()
    measures$PROBLEM_DIM <- smoof::getNumberOfParameters(obj.fn)
    measures$PROBLEM_OBJ <- smoof::getNumberOfObjectives(obj.fn)
    measures$NUM_EVALS <- smoof::getNumberOfEvaluations(obj.fn)

    # Prepare data
    archive <- t(data.matrix(smoof::getLoggedValues(obj.fn)$pars))
    archive_performance <- data.matrix(smoof::getLoggedValues(obj.fn)$obj.vals)
    archive_nd_mask <- ecr3vis::nondominated(archive_performance)

    population <- t(data.matrix(population))
    population_performance <- apply(population, 2, obj.fn)
    population_nd_mask <- ecr3vis::nondominated(population_performance)

    #Get reference data: point and front
    load("refdata.RData") #TODO file for each function since we will get a lot more...

    instance_name <- tail(unlist(strsplit(instance_path, "/")), n = 1)

    reference.point <- smoof::getRefPoint(fn)
    if (is.null(reference.point)) {
        reference.point <- references[[instance_name]]$newrefpoint
    }
    reference.hv <- references[[instance_name]]$newhv

    # cat(instance_name, '\n')
    # cat('newhv:', references[[instance_name]]$newhv, '\n')

    # POPULATION CODE
    measures$POP_ND_SIZE <- length(which(population_nd_mask))
    measures$POP_SAMPLED <- FALSE

    measures$POP_SP <- -ecr3vis::solow_polasky(population)
    if(measures$POP_ND_SIZE == 1){
        measures$POP_HV <- -prod(reference.point - population_performance[, population_nd_mask])
        measures$POP_ND_SP <- -1
    }
    else {
        if(measures$POP_ND_SIZE > 2000){
            # SAMPLE ND POINTS
            measures$POP_SAMPLED <- TRUE
            sampled_nd_indices <-sample(which(population_nd_mask), 2000, replace = FALSE)
            population_nd_mask <- rep(FALSE, times=length(population_nd_mask))
            population_nd_mask[sampled_nd_indices] <- TRUE
        }

        measures$POP_HV <- -ecr3vis::hv(population_performance[, population_nd_mask], reference.point)
        measures$POP_ND_SP <- -ecr3vis::solow_polasky(population[, population_nd_mask])
    }
    measures$POP_HV_NORM <- measures$POP_HV / reference.hv

    # ARCHIVE CODE
    measures$ARC_ND_SIZE <- length(which(archive_nd_mask))
    measures$ARC_SAMPLED <- FALSE
    if(measures$ARC_ND_SIZE == 1){
        measures$ARC_HV <- -prod(reference.point - archive_performance[, archive_nd_mask])
        measures$ARC_ND_SP <- -1
    }
    else {
        if(measures$ARC_ND_SIZE > 2000){
            # SAMPLE ND POINTS
            measures$ARC_SAMPLED <- TRUE
            sampled_nd_indices <-sample(which(archive_nd_mask), 2000, replace = FALSE)
            archive_nd_mask <- rep(FALSE, times=length(archive_nd_mask))
            archive_nd_mask[sampled_nd_indices] <- TRUE

            # cat("nd samples =", paste(length(which(archive_nd_mask == TRUE))), "\n")
        }

        measures$ARC_HV <- -ecr3vis::hv(archive_performance[, archive_nd_mask], reference.point)
        measures$ARC_ND_SP <- -ecr3vis::solow_polasky(archive[, archive_nd_mask])
    }

    # Normalise HV
    measures$ARC_HV_NORM <- measures$ARC_HV / reference.hv

    # # IDG+ MINIMISE
    # measures$IGDP <- NULL # ecr3vis::igdp(pareto.matrix, reference.front) #JEROEN: Not used so do not waste resources
    # measures$ABSE <- NULL
    return(measures)

    # if (!is.null(opt$save_solution)) {
    #     writeLines("Save to file")
    #     # save(solution_set, file=opt$save_solution)
    #     measuresdf <- data.frame(Reduce(rbind, measures))
    #
    #     HV <- measures$HV
    #     HVN <- measures$HVN
    #     IGDP <- measures$IGDP
    #     SP <- measures$SP
    #     SPD <- measures$SPD
    #
    #     # save(pareto.matrix, populationnd, measuresdf , HV, SP, SPD , IGDP, reference.point, file=opt$save_solution)
    #     save(HV, SP, SPD, HVN, file = opt$save_solution)
    # }
    #
    # if (!is.null(opt$visualise)) {
    #     output <- opt$visualise
    #     pdf(output)
    #     # print(dim(as.data.frame(t(population))))
    #     # print(dim(as.data.frame(t(solution_set))))
    #     # print(dim(as.data.frame(t(pareto.matrix))))
    #     # plot(t(apply(population, 2, obj.fn)), main="Pop -> Obj")
    #     # plot(t(apply(populationnd, 2, obj.fn)), main="Pop ND -> Obj")
    #
    #     plot(t(population), main = "Decision space")
    #     plot(t(population_performance), main = "Objective space")
    #     plot(t(pareto.matrix), main = "Non-dominated set in objective space")
    #     dev.off()
    #
    #     output <- paste0(opt$visualise, ".Rdata")
    #     save(populationnd, pareto.matrix, file = output)
    # }
    # writeLines(paste0("s REFERENCE", paste(reference.point, collapse = ","), " POPSIZE (", toString(dim(population)), ") NON-DOMINATED POP (", toString(dim(populationnd)), ") NON-DOMINATED OBJ (", toString(dim(pareto.matrix)), ")"))
    # return(measures)
}

process_results <- function(population, fn, opt){
    measures <- compute_performance_metrics(population, fn, opt$instance)

    if(!is.null(opt$output)){
        writeLines(toJSON(as.vector(measures), pretty = TRUE, auto_unbox = TRUE ), opt$output)
        cat("OUTPUT WRITTEN TO: ", opt$output ,"\n")
    }


}

makeBiObjMPM2Function <- function(
    dimensions = 2L,
    env = "R",
    peaks_1 = 10L, topology_1 = "random", seed_1 = 1L, rotated_1 = TRUE, peak_shape_1 = "ellipse",
    peaks_2 = 1000L, topology_2 = "random", seed_2 = 1001L, rotated_2 = TRUE, peak_shape_2 = "ellipse") {
    fn1 <- smoof::makeMPM2Function(peaks_1, dimensions, topology_1, seed_1, evaluation.env = env)
    fn2 <- smoof::makeMPM2Function(peaks_2, dimensions, topology_2, seed_2, evaluation.env = env)

    smoof::makeMultiObjectiveFunction(
        name = paste0(topology_1, "-", topology_2, "_", peaks_1, "-", peaks_2, "_", "instance_", seed_1, "-", seed_2, "_", dimensions, "D"),
        id = paste0(topology_1, "_", peaks_1, "_", "instance_", seed_1, "_", dimensions, "D"),
        minimize = c(TRUE, TRUE),
        fn = function(x) {
            c(
                fn1(x),
                fn2(x)
            )
        },
        n.objectives = 2L,
        par.set = ParamHelpers::makeNumericParamSet("x", lower = rep(-0.2, dimensions), upper = rep(1.2, dimensions))
    )
}

makeCustomBiObjBBOBFunction <- function(dimensions = 2L, fids = c(1, 1), iids = c(1, 1)) {
    par.set <- makeNumericParamSet("x", len = dimensions, lower = -5, upper = 5)

    fid1 <- fids[[1]]
    fid2 <- fids[[2]]

    iid1 <- iids[[1]]
    iid2 <- iids[[2]]

    smoof::makeMultiObjectiveFunction(
        name = sprintf("Bi-Objective BBOB_%i_%i_%i_%i_%i", dimensions, fid1, fid2, iid1, iid2),
        id = paste0("biobj_bbob_", dimensions, "d_2o"),
        description = sprintf(
            "%i,%i-th noiseless Bi-Objective BBOB function\n(FID: %i,%i, IID: %i,%i, DIMENSION: %i)",
            fid1, fid2, fid1, fid2, iid1, iid2, dimensions
        ),
        minimize = c(TRUE, TRUE),
        fn = function(x) {
            c(
                .Call("evaluateBBOBFunctionCPP", dimensions, fid1, iid1, x),
                .Call("evaluateBBOBFunctionCPP", dimensions, fid2, iid2, x)
            )
        },
        par.set = par.set,
        n.objectives = 2L,
        vectorized = FALSE
    )
}


makeBiObjBBBoo <- function(dimensions = 2L,
                           fid = 1L, iid = 1L,
                           peaks = 1L, topology = "random", seed = 42L, rotated = TRUE, peak_shape = "ellipse", env = "R") {
    par.set <- makeNumericParamSet("x", len = dimensions, lower = 0, upper = 1)
    fn1 <- smoof::makeMPM2Function(peaks, dimensions, topology, seed, evaluation.env = env)
    fn2 <- smoof::makeBBOBFunction(dimensions, fid, iid)

    smoof::makeMultiObjectiveFunction(
        name = sprintf("Bi-ObjBBBoo_dim-%i_fid-%i_iid-%i_peaks-%i_topology-%s", dimensions, fid, iid, peaks, topology),
        id = sprintf("Bi-ObjBBBoo_dim%i_fid%i_iid%i_peaks%i_topology%s", dimensions, fid, iid, peaks, topology),
        description = sprintf(
            "BiObjBBBoo"
        ),
        minimize = c(TRUE, TRUE),
        fn = function(x) {
            xBBOB <- x * 10 - 5
            c(
                fn1(x),
                fn2(xBBOB)
            )
        },
        par.set = par.set,
        n.objectives = 2L,
        # the single-objective BBOB functions are vectorized,
        # but not the combined one
        vectorized = FALSE
    )
}

# # necessary to load some functions
# smoof::makeMPM2Function(1, 1, "random", 42, TRUE, "ellipse")
#
#
# print_and_save_solution_set <- function(solution_set) {
#     writeLines("s SOLUTION SET (not showed)")
#     print(solution_set)
#     if (!is.null(opt$save_solution)) {
#         writeLines("Save to file")
#         save(solution_set, file = opt$save_solution)
#         # write.table(solution_set, file = opt$save_solution)
#     }
# }
#
# compute_performance_metrics <- function(population, solution_set, fn, instance_path) {
#     # Get reference data
#     population <- t(data.matrix(population))
#     popnondom <- nondominated(apply(population, 2, obj.fn))
#     populationnd <- population[, popnondom] # Filter non dominated set
#
#
#
#     solution_set <- t(data.matrix(solution_set)) # No guarantee of non-domination
#     instance_name <- tail(unlist(strsplit(instance_path, "/")), n = 1)
#
#     load("refdata.RData")
#
#     # reference.front <- references[[instance_name]]$approxfront
#     # reference.front <- t(data.matrix(reference.front))
#
#     reference.point <- smoof::getRefPoint(fn)
#     newrefpoint <- smoof::getRefPoint(fn)
#     if (is.null(reference.point)) {
#         reference.point <- references[[instance_name]]$refpoint
#         newrefpoint <- references[[instance_name]]$newrefpoint
#     }
#
#     newoffset <- references[[instance_name]]$newoffset
#     newhv <- references[[instance_name]]$newhv
#
#     # print(population)
#     # print(solution_set)
#
#     # Compute measures
#     measures <- list()
#     # HV MAXIMISE, hence minimise
#     # refpoint=pareto.refpoint, newrefpoint=newrefpoint, newoffset=newoffset, newfront=newfront
#     # Only use non-dominated set:
#     pareto.matrix <- solution_set[, ecr::nondominated(solution_set)]
#     print(pareto.matrix)
#     if (is.matrix(pareto.matrix)) {
#         if (ncol(pareto.matrix) > 2000) pareto.matrix <- pareto.matrix[, sample(ncol(pareto.matrix), 2000)]
#         measures$HV <- -ecr3vis::hv(pareto.matrix, reference.point)
#         measures$HVN <- -ecr3vis::hv(pareto.matrix + newoffset, newrefpoint)
#         measures$SP <- -ecr3vis::solow_polasky(pareto.matrix)
#     } else {
#         if (sum(reference.point < pareto.matrix) == length(reference.point)) {
#             measures$HV <- -prod(reference.point - pareto.matrix)
#             measures$HVN <- -prod(newrefpoint - (pareto.matrix + newoffset))
#         } else {
#             measure$HV <- 0
#             measure$HVN <- 0
#         }
#         measures$SP <- -1
#     }
#     if (is.matrix(populationnd)) {
#         if (ncol(populationnd) > 2000) populationnd <- populationnd[, sample(ncol(populationnd), 2000)]
#         measures$SPD <- -ecr3vis::solow_polasky(populationnd)
#     } else {
#         measures$SPD <- -1
#     }
#
#     measures$HVN <- measures$HVN / newhv # Normalized
#
#     # IDG+ MINIMISE
#     measures$IGDP <- NULL # ecr3vis::igdp(pareto.matrix, reference.front) #JEROEN: Not used so do not waste resources
#     # TODO: implement Approach for Basin Separated Evaluation
#     measures$ABSE <- NULL
#
#     if (!is.null(opt$save_solution)) {
#         writeLines("Save to file")
#         # save(solution_set, file=opt$save_solution)
#         measuresdf <- data.frame(Reduce(rbind, measures))
#
#         HV <- measures$HV
#         HVN <- measures$HVN
#         IGDP <- measures$IGDP
#         SP <- measures$SP
#         SPD <- measures$SPD
#
#         # save(pareto.matrix, populationnd, measuresdf , HV, SP, SPD , IGDP, reference.point, file=opt$save_solution)
#         save(HV, SP, SPD, HVN, file = opt$save_solution)
#     }
#
#     if (!is.null(opt$visualise)) {
#         output <- opt$visualise
#         pdf(output)
#         # print(dim(as.data.frame(t(population))))
#         # print(dim(as.data.frame(t(solution_set))))
#         # print(dim(as.data.frame(t(pareto.matrix))))
#         # plot(t(apply(population, 2, obj.fn)), main="Pop -> Obj")
#         # plot(t(apply(populationnd, 2, obj.fn)), main="Pop ND -> Obj")
#
#         plot(t(population), main = "Decision space")
#         plot(t(solution_set), main = "Objective space")
#         plot(t(pareto.matrix), main = "Non-dominated set in objective space")
#         dev.off()
#
#         output <- paste0(opt$visualise, ".Rdata")
#         save(populationnd, pareto.matrix, file = output)
#     }
#     writeLines(paste0("s REFERENCE", paste(reference.point, collapse = ","), " POPSIZE (", toString(dim(population)), ") NON-DOMINATED POP (", toString(dim(populationnd)), ") NON-DOMINATED OBJ (", toString(dim(pareto.matrix)), ")"))
#     return(measures)
# }
#
# print_measures <- function(measures) {
#     writeLines("s MEASURES")
#     writeLines(paste("s HV", as.character(measures$HV)))
#     writeLines(paste("s IGDP", as.character(measures$IGDP)))
#     writeLines(paste("s SP", as.character(measures$SP)))
#     writeLines(paste("s SPD", as.character(measures$SPD)))
#     writeLines(paste("s HVN", as.character(measures$HVN)))
# }
#
# plot_solutions <- function(solution_set, fn, instance_path) {
#     measures <- compute_performance_metrics(solution_set, fn, instance_path)
#
#     load("refdata.RData")
#
#     reference.front <- references[[instance_name]]$approxfront
#     reference.front <- t(data.matrix(reference.front))
#
#     reference.point <- smoof::getRefPoint(fn)
# }
#
# # Peek-A-Boo! function stuff
#
# get_peak_metadata <- function(npeaks, dimension, topology,
#                               randomSeed, rotated, peakShape) {
#     xopt <- getAllPeaks(
#         npeaks, dimension, topology,
#         randomSeed, rotated, peakShape
#     )
#
#     colnames(xopt) <- paste0("x", 1:dimension)
#
#     cov <- getCovarianceMatrices(
#         npeaks, dimension, topology,
#         randomSeed, rotated, peakShape
#     ) %>%
#         lapply(function(x) matrix(x[[1]], nrow = dimension, ncol = dimension))
#
#     height <- getAllHeights(
#         npeaks, dimension, topology,
#         randomSeed, rotated, peakShape
#     )
#
#     shape <- getAllShapes(
#         npeaks, dimension, topology,
#         randomSeed, rotated, peakShape
#     )
#
#     radius <- getAllRadii(
#         npeaks, dimension, topology,
#         randomSeed, rotated, peakShape
#     )
#
#     peak_fns <- lapply(1:nrow(xopt), function(i) {
#         create_peak_function(cov[[i]], xopt[i, ], height[i], shape[i], radius[i])
#     })
#
#     fn <- function(x) {
#         min(sapply(peak_fns, function(f) f(x)))
#     }
#
#     list(
#         xopt = xopt,
#         cov = cov,
#         height = height,
#         shape = shape,
#         radius = radius,
#         peak_fns = peak_fns,
#         fn = fn
#     )
# }
#
# create_peak_function <- function(cov, xopt, height, shape, radius) {
#     function(x) {
#         if (is.matrix(x)) {
#             dx <- t(apply(x, 1, function(row) (row - xopt))) %*% chol(cov)
#             md <- apply(dx, 1, function(row) sqrt(sum(row**2)))
#         } else {
#             md <- sqrt(t(x - xopt) %*% cov %*% (x - xopt))
#             # dx <- (x - xopt) %*% chol(cov)
#             # md <- sqrt(sum(dx**2))
#         }
#         g <- height / (1 + md**shape / radius)
#         return(1 - g)
#     }
# }
#
#
# makeBiObjMPM2Function <- function(
#     dimensions = 2,
#     peaks_1 = 10, topology_1 = "random", seed_1 = 42, rotated_1 = TRUE, peak_shape_1 = "ellipse",
#     peaks_2 = 1000, topology_2 = "random", seed_2 = 42, rotated_2 = TRUE, peak_shape_2 = "ellipse") {
#     peak_data_1 <- get_peak_metadata(
#         dimension = dimensions,
#         npeaks = peaks_1,
#         topology = topology_1,
#         randomSeed = seed_1,
#         rotated = rotated_1,
#         peakShape = peak_shape_1
#     )
#     peak_data_2 <- get_peak_metadata(
#         dimension = dimensions,
#         npeaks = peaks_2,
#         topology = topology_2,
#         randomSeed = seed_2,
#         rotated = rotated_2,
#         peakShape = peak_shape_2
#     )
#
#     fn <- smoof::makeMultiObjectiveFunction(
#         name = paste0(topology_1, "_", peaks_1, "_", "instance_", seed_1, "_", dimensions, "D"),
#         id = paste0(topology_1, "_", peaks_1, "_", "instance_", seed_1, "_", dimensions, "D"),
#         fn = function(x) {
#             c(
#                 peak_data_1$fn(x),
#                 peak_data_2$fn(x)
#             )
#         },
#         par.set = ParamHelpers::makeNumericParamSet("x", lower = rep(-0.2, dimensions), upper = rep(1.2, dimensions))
#     )
# }

#include <Rcpp.h>
#include <vector>
#include <sstream>
#include <map>

#include "RcppUtilities.h"
#include "commons/globals.h"
#include "forest/ForestTrainers.h"
#include "forest/ForestPredictors.h"

// NOW LOCALLY LINEAR FOREST BINDINGS

// [[Rcpp::export]]
Rcpp::List locally_linear_train(Rcpp::NumericMatrix input_data,
                            size_t outcome_index,
                            Rcpp::RawMatrix sparse_data,
                            std::vector <std::string> variable_names,
                            unsigned int mtry,
                            unsigned int num_trees,
                            bool verbose,
                            unsigned int num_threads,
                            unsigned int min_node_size,
                            bool sample_with_replacement,
                            bool keep_inbag,
                            double sample_fraction,
                            std::vector<size_t> no_split_variables,
                            unsigned int seed,
                            bool honesty,
                            unsigned int ci_group_size,
                            double lambda) {
 
  Data* data = RcppUtilities::convert_data(input_data, sparse_data, variable_names);
    
  ForestTrainer trainer = ForestTrainers::locally_linear_trainer(data, outcome_index, lambda);
  RcppUtilities::initialize_trainer(trainer, mtry, num_trees, num_threads, min_node_size,
      sample_with_replacement, sample_fraction, no_split_variables, seed, honesty, ci_group_size);
    
  Forest forest = trainer.train(data);

  Rcpp::List result;
  Rcpp::RawVector serialized_forest = RcppUtilities::serialize_forest(forest);
  result.push_back(serialized_forest, RcppUtilities::SERIALIZED_FOREST_KEY);
  result.push_back(forest.get_trees().size(), "num.trees");

  delete data;

  return result;
}

// [[Rcpp::export]]
Rcpp::NumericMatrix locally_linear_predict(Rcpp::List forest,
                                       Rcpp::NumericMatrix input_data,
                                       Rcpp::RawMatrix sparse_data,
                                       std::vector<std::string> variable_names,
                                       unsigned int num_threads,
                                       double lambda) {
    
  std::cout << "inside bindings. beginning predict.   ";
  Data *data = RcppUtilities::convert_data(input_data, sparse_data, variable_names);
    
  Forest deserialized_forest = RcppUtilities::deserialize_forest(
      forest[RcppUtilities::SERIALIZED_FOREST_KEY]);

  ForestPredictor predictor = ForestPredictors::locally_linear_predictor(num_threads, data, lambda);
    
  std::cout << "bindings marker 1.   ";
    
  std::vector<Prediction> predictions = predictor.predict(deserialized_forest, data); //***

  std::cout << "bindings marker 2.   ";
    
  Rcpp::NumericMatrix result = RcppUtilities::create_prediction_matrix(predictions);
    
  std::cout << "bindings marker 3.   ";

  delete data;
  return result;
}

// [[Rcpp::export]]
Rcpp::NumericMatrix locally_linear_predict_oob(Rcpp::List forest,
                                           Rcpp::NumericMatrix input_data,
                                           Rcpp::RawMatrix sparse_data,
                                           std::vector<std::string> variable_names,
                                           unsigned int num_threads,
                                           double lambda) {
  Data *data = RcppUtilities::convert_data(input_data, sparse_data, variable_names);
  Forest deserialized_forest = RcppUtilities::deserialize_forest(
      forest[RcppUtilities::SERIALIZED_FOREST_KEY]);

  ForestPredictor predictor = ForestPredictors::locally_linear_predictor(num_threads, data, lambda);
  std::vector<Prediction> predictions = predictor.predict_oob(deserialized_forest, data);
  Rcpp::NumericMatrix result = RcppUtilities::create_prediction_matrix(predictions);

  delete data;
  return result;
}

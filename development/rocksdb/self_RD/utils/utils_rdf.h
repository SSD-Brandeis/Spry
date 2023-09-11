
#ifndef UTILS_RDF_H
#define UTILS_RDF_H

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "rocksdb/db.h"
#include "rocksdb/options.h"
#include <rocksdb/sys_rdfilter.h>


void retrieve_all_RDFs(DB** dp_ptr2, 
                       PLRDF plrdf_prime, 
                       PLRDF split_plrdf_prime, 
                       PLRDF top_level_rdf_prime, 
                       std::vector<t3ll> skyline_rdf_prime, 
                       std::vector<int> skyline__numbers_of_ranges_in_rdf_log){
  DB* db = *dp_ptr2;
  // retrieve all RDFs
  plrdf_prime = *(db->getPLRDF());
  split_plrdf_prime = *(db->getSplitPLRDF());
  top_level_rdf_prime = *(db->getTopLevelRDF());
  skyline_rdf_prime = *(db->getSkylineRDF());
  skyline__numbers_of_ranges_in_rdf_log = *(db->getSkylineNumbersOfRangesInRDFLog());
}

void set_all_RDFs(DB** dp_ptr2, 
                  PLRDF &plrdf_prime, 
                  PLRDF &split_plrdf_prime, 
                  PLRDF &top_level_rdf_prime, 
                  std::vector<t3ll> &skyline_rdf_prime, 
                  std::vector<int> &skyline__numbers_of_ranges_in_rdf_log){
  DB* db = *dp_ptr2;
  // set all RDFs
  // db->setPLRDF(std::vector<int>());
  db->setPLRDF(&plrdf_prime);
  db->setSplitPLRDF(&split_plrdf_prime);
  db->setTopLevelRDF(&top_level_rdf_prime);
  db->setSkylineRDF(&skyline_rdf_prime);
  db->setSkylineNumbersOfRangesInRDFLog(&skyline__numbers_of_ranges_in_rdf_log);
}

#endif // UTILS_RDF_H
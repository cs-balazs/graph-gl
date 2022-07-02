#ifndef COMPUTE_TREE_HEADER
#define COMPUTE_TREE_HEADER

#include <stdlib.h>

#define FORMULA_MAX_LENGTH 200

typedef char Formula[FORMULA_MAX_LENGTH];

// TODO: enums probably are unnecessary

typedef enum Operator {
  PLUS = '+',
  MINUS = '-',
  TIMES = '*',
  DIVISION = '/',
  POWER = '^',
  OPENING_BRACKET = '(',
  CLOSING_BRACKET = ')',
} Operator;

typedef enum OperatorWeight {
  W_PLUS = 0,
  W_MINUS = 0,
  W_TIMES = 1,
  W_DIVISION = 1,
  W_POWER = 2,
  W_OPENING_BRACKET = 3,
  W_CLOSING_BRACKET = -3,
} OperatorWeight;

typedef union NodeData {
  Operator operator;
  double constant;
} NodeData;

typedef enum NodeType {
  OPERATOR,
  VARIABLE,
  CONSTANT,
} NodeType;

typedef struct ParseTree {
  struct ParseTree *parent;
  struct ParseTree *left;
  struct ParseTree *right;
  NodeType type;
  NodeData data;
} ParseTree;

ParseTree *parse_formula(Formula formula);
void free_parse_tree(ParseTree *tree);
double compute(ParseTree *tree, double x);
void mermaid_export(ParseTree *tree);

#endif
#include "parse_tree.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "io.h"

/**
 * TODOS:
 *   + Remove brackets from formula after calculating the weights (also shift the weights char[])
 *   - Drag to move in space
 *   - Scroll to scale
 *   - Reset frame to origin
 *   - Draw a grid (with numbers?)
 *   - Support spaces (remove them like brackets)
 *   - Support "2x" instead of 2*x (also similar transformation to removal)
 *   - free stuff allocated on heap
 *   - Support negative numbers, currentls '-' is being interpreted as minus operator, so (0 - 3) is -3 now
 *   - Support some unary operators like: sin() cos(), log(), ! (factorial)
 *       - Shouldn't be too hard, either left or right will be NULL
 *   + Mermaid export of the tree
 */

ParseTree *construct_tree_from_formula(Formula formula, Formula weights,
                                       uint8_t start, uint8_t end,
                                       uint8_t formula_length,
                                       uint8_t highest_weight,
                                       ParseTree *parent)
{
  uint8_t rightmost_smallest_weight_operator_index = 0;
  uint8_t found_operator = 0;
  uint8_t smallest_weight = 0;
  uint8_t found_variable = 0;

  for (; smallest_weight <= highest_weight && found_operator != 1;
       smallest_weight++) {
    for (int16_t i = end - 1; i >= start; i--) {
      if (formula[i] == 'x') {
        found_variable = 1;
      }
      if (weights[i] == smallest_weight) {
        rightmost_smallest_weight_operator_index = i;
        found_operator = 1;
        break;
      }
    }
  }

  if (found_operator) {
    ParseTree *node = malloc(sizeof(ParseTree));
    node->parent = parent;
    node->type = OPERATOR;
    node->data =
      (NodeData){ operator: formula[rightmost_smallest_weight_operator_index] };

    node->left = construct_tree_from_formula(
      formula, weights, start, rightmost_smallest_weight_operator_index,
      rightmost_smallest_weight_operator_index, highest_weight, node);

    node->right = construct_tree_from_formula(
      formula, weights, rightmost_smallest_weight_operator_index + 1, end,
      formula_length - rightmost_smallest_weight_operator_index - 1,
      highest_weight, node);

    return node;
  } else if (found_variable == 1 ||
             (formula_length == 1 && formula[start] == 'x')) {
    ParseTree *node = malloc(sizeof(ParseTree));
    node->parent = parent;
    node->type = VARIABLE;
    node->left = NULL;
    node->right = NULL;
    return node;
  } else {
    Formula only_constant_part = { 0 };
    strncpy(only_constant_part, formula + start, end - start);
    double constant;
    sscanf(only_constant_part, "%lf", &constant);
    ParseTree *node = malloc(sizeof(ParseTree));
    node->parent = parent;
    node->type = CONSTANT;
    node->left = NULL;
    node->right = NULL;
    // NodeData data;
    // data.constant = constant;
    node->data = (NodeData){ constant: constant };
    return node;
  }
}

ParseTree *parse_formula(Formula formula)
{
  Formula weights;
  for (uint8_t i = 0; i < FORMULA_MAX_LENGTH; i++) {
    weights[i] = -1;
  }

  uint16_t extra_weight = 0; // Count extra weight of brackets

  uint8_t highest_weight = 0;
  uint8_t formula_length = 0;
  for (; formula[formula_length] != 0; formula_length++) {
    // -1 init and "1 +" to differentiate if there was a case match
    int8_t new_weight = -1;
    switch (formula[formula_length]) {
    case TIMES:
    case DIVISION:
      new_weight = 1 + W_TIMES + extra_weight;
      break;
    case PLUS:
    case MINUS:
      new_weight = 1 + W_PLUS + extra_weight; // == W_MINUS
      break;
      new_weight = 1 + W_TIMES + extra_weight; // == W_DIVISION
      break;
    case POWER:
      new_weight = 1 + W_POWER + extra_weight;
      break;

    case OPENING_BRACKET:
      extra_weight += W_OPENING_BRACKET;
      break;
    case CLOSING_BRACKET:
      extra_weight += W_CLOSING_BRACKET;
      break;
    }

    if (new_weight > 0) {
      new_weight--;
      weights[formula_length] = new_weight;
    }

    if (new_weight > highest_weight) {
      highest_weight = new_weight;
    }
  }

  uint8_t count_of_found_brackets = 0;

  for (uint8_t i = 0; formula[i] != 0;) {
    if (formula[i] == '(' || formula[i] == ')') {
      memmove(weights + i, weights + i + 1,
              formula_length - i + count_of_found_brackets);
      memmove(formula + i, formula + i + 1,
              formula_length - i + count_of_found_brackets);
      count_of_found_brackets++;
    } else {
      i++;
    }
  }

  formula_length -= count_of_found_brackets;

  ParseTree *node = construct_tree_from_formula(
    formula, weights, 0, formula_length, formula_length, highest_weight, NULL);

  return node;
}

double compute(ParseTree *tree, double x)
{
  switch (tree->type) {
  case CONSTANT:
    return tree->data.constant;
  case VARIABLE:
    return x;
  case OPERATOR:
    switch (tree->data.operator) {
    case PLUS:
      return compute(tree->left, x) + compute(tree->right, x);
    case MINUS:
      return compute(tree->left, x) - compute(tree->right, x);
    case TIMES:
      return compute(tree->left, x) * compute(tree->right, x);
    case DIVISION:
      return compute(tree->left, x) / compute(tree->right, x);
    case POWER:
      return pow(compute(tree->left, x), compute(tree->right, x));
    }
  }
}

void free_parse_tree(ParseTree *tree)
{
}

void mermaid_node(ParseTree *node, char *html)
{
  char id[20] = { 0 };
  // Somewhat hacky solution here, using the memory address of the node as Mermaid ID
  sprintf(id, "%p", node);

  strcat(html, id);
  strcat(html, "((");
  switch (node->type) {
  case OPERATOR:
    char op[4] = { '\"', (char)node->data.operator, '\"', 0 };
    strcat(html, op);
    break;

  case VARIABLE:
    strcat(html, "x");
    break;

  case CONSTANT:
    char constant_str[100] = { 0 };
    sprintf(constant_str, "%lf", node->data.constant);
    strcat(html, constant_str);
    break;
  }
  strcat(html, "))");
}

void tree_to_mermaid(ParseTree *tree, char *html)
{
  if (tree->parent != NULL) {
    strcat(html, "  ");
    mermaid_node(tree->parent, html);
    strcat(html, " --- ");
    mermaid_node(tree, html);
    strcat(html, "\n");
  }
  if (tree->left != NULL) {
    tree_to_mermaid(tree->left, html);
  }
  if (tree->right != NULL) {
    tree_to_mermaid(tree->right, html);
  }
}

void mermaid_export(ParseTree *tree)
{
  char html[5000] =
    "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><title>Parse Tree</title></head><body><script src=\"https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js\"></script><div class=\"mermaid\">\ngraph TD\n  ";
  if (tree->left == NULL && tree->right == NULL) {
    mermaid_node(tree, html);
  }
  tree_to_mermaid(tree, html);
  strcat(html, "</div></body></html>");
  write_file("mermaid.html", html);
}
#include "parse_tree.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

// #define DEBUG

#ifdef DEBUG
#define DEBUG_LOG(a) a
#endif

#ifndef DEBUG
#define DEBUG_LOG(a) ;
#endif

/**
 * TODOS:
 *   + Remove brackets from formula after calculating the weights (also shift the weights char[])
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
  DEBUG_LOG(
    printf(
      "-------\nconstruct_tree_from_formula(\n  formula = %s,\n  start = %d\n  end =%d\n  formula_length = %d,\n  highest_weight = %d,\n  weights = ",
      formula, start, end, formula_length, highest_weight);
    for (uint8_t i = 0; formula[i] != 0; i++) {
      if (weights[i] >= 0) {
        printf("%d", weights[i]);
      } else {
        printf(" ", weights[i]);
      }
    } printf(",\n)\n"));

  uint8_t rightmost_smallest_weight_operator_index = 0;
  uint8_t found_operator = 0;
  uint8_t smallest_weight = 0;
  uint8_t found_variable = 0;

  for (; smallest_weight <= highest_weight && found_operator != 1;
       smallest_weight++) {
    for (int16_t i = end - 1; i >= start; i--) {
      DEBUG_LOG(printf("i = %d\n", i));
      DEBUG_LOG(printf("Checking if %c is equal to %c\n", formula[i], 'x'));
      if (formula[i] == 'x') {
        found_variable = 1;
      }
      if (weights[i] == smallest_weight) {
        DEBUG_LOG(printf(
          "setting rightmost_smallest_weight_operator_index to %d\n", i));
        rightmost_smallest_weight_operator_index = i;
        found_operator = 1;
        break;
      }
    }
  }
  DEBUG_LOG(printf("found_variable = %d", found_variable));
  DEBUG_LOG(printf("smallest_weight = %d\n", smallest_weight);
            printf("rightmost_smallest_weight_operator_index = %d\n",
                   rightmost_smallest_weight_operator_index));

  if (found_operator) {
    DEBUG_LOG(printf("Type: FORMULA\n"));

    ParseTree *node = malloc(sizeof(ParseTree));
    node->parent = parent;
    node->type = OPERATOR;
    node->data =
      (NodeData){ operator: formula[rightmost_smallest_weight_operator_index] };

    DEBUG_LOG(printf("LEFT CALL: start: %d, end: %d\n\n", start,
                     rightmost_smallest_weight_operator_index));
    DEBUG_LOG(printf("RIGHT CALL: start: %d, end: %d\n\n",
                     rightmost_smallest_weight_operator_index + 1, end));

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
    DEBUG_LOG(printf("Type: VARIABLE\n"));
    ParseTree *node = malloc(sizeof(ParseTree));
    node->parent = parent;
    node->type = VARIABLE;
    node->left = NULL;
    node->right = NULL;
    return node;
  } else {
    DEBUG_LOG(printf("Type: CONSTANT\n"));
    Formula only_constant_part = { 0 };
    strncpy(only_constant_part, formula + start, end - start);
    DEBUG_LOG(printf("only_constant_part = %s\n", only_constant_part));
    double constant;
    sscanf(only_constant_part, "%lf", &constant);
    ParseTree *node = malloc(sizeof(ParseTree));
    node->parent = parent;
    node->type = CONSTANT;
    node->left = NULL;
    node->right = NULL;
    NodeData data;
    data.constant = constant;
    node->data = (NodeData){ constant: constant };
    printf("constant: %lf\n", constant);
    printf("data.constant = %lf\n", data.constant);
    printf("node->data.constant = %lf\n", node->data.constant);
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

  DEBUG_LOG(printf("%s\n", formula);

            for (uint8_t i = 0; formula[i] != 0; i++) {
              if (weights[i] >= 0) {
                printf("%d", weights[i]);
              } else {
                printf(" ", weights[i]);
              }
            } putchar('\n'));

  for (uint8_t i = 0; formula[i] != 0;) {
    if (formula[i] == '(' || formula[i] == ')') {
      // strcpy(formula + i, formula + i + 1);
      // strcpy(weights + i, weights + i + 1);

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

  DEBUG_LOG(printf("%s\n", formula);

            for (uint8_t i = 0; formula[i] != 0; i++) {
              if (weights[i] >= 0) {
                printf("%d", weights[i]);
              } else {
                printf(" ", weights[i]);
              }
            } putchar('\n'));

  ParseTree *node = construct_tree_from_formula(
    formula, weights, 0, formula_length, formula_length, highest_weight, NULL);

  return node;
}

double compute(ParseTree *tree, double x)
{
}

void free_parse_tree(ParseTree *tree)
{
}

void mermaid_node(ParseTree *node, char *html)
{
  printf("mermaid_node called\n");

  char id[20] = { 0 };
  // Somewhat hacky solution here, using the memory address of the node as Mermaid ID
  sprintf(id, "%p", node);

  printf("id = %s\n", id);

  strcat(html, id);
  printf("strcat(html, id) successful\n");
  strcat(html, "((");
  printf("strcat(html, \"((\") successful\n");
  switch (node->type) {
  case OPERATOR:
    printf("OPERATOR\n");
    printf("%c\n", (char)node->data.operator);
    char op[4] = { '\"', (char)node->data.operator, '\"', 0 };
    strcat(html, op);
    printf("strcat(html, (char)node->data.operator) successful\n");
    break;

  case VARIABLE:
    printf("VARIABLE\n");
    strcat(html, "x");
    printf("strcat(html, \"x\") successful\n");
    break;

  case CONSTANT:
    printf("CONSTANT\n");
    printf("node->data.constant = %lf\n", node->data.constant);
    char constant_str[100] = { 0 };
    sprintf(constant_str, "%lf", node->data.constant);
    strcat(html, constant_str);
    printf("strcat(html, constant_str) successful\n");
    break;
  }
  strcat(html, "))");
  printf("strcat(html, \"))\") successful\n");
}

void tree_to_mermaid(ParseTree *tree, char *html)
{
  printf("tree_to_mermaid called\n");
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
  printf("mermaid_export called\n");
  char html[5000] =
    "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\" /><title>Parse Tree</title></head><body><script src=\"https://cdn.jsdelivr.net/npm/mermaid/dist/mermaid.min.js\"></script><div class=\"mermaid\">\ngraph TD\n";

  printf("html = %s\n", html);

  tree_to_mermaid(tree, html);
  printf("html = %s\n", html);

  strcat(html, "</div></body></html>");
  printf("html = %s\n", html);
  write_file("index.html", html);
}
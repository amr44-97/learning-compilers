#pragma once


struct Node;

enum class NodeType {
    Identifier,
    StringLiteral,
    NumberLiteral,
    Add,
    Sub,
    Mul,
    Div,
};



struct Node{
    NodeType type;
    Node** children;
};

Node* create_node(NodeType type,Node* lhs, Node* rhs);

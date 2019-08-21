#include <stdio.h>

#include "optimizer.h"
#include "runtime.h"

static u64 next_label = 0;

static void optimize_execs(Allocator, Array<Node*> *code) {
    u32 i = 0;
    while(i < code->count) {
        if(i + 1 < code->count &&
            code->data[i]->type == NODE_LAMBDA &&
            code->data[i + 1]->type == NODE_INSTRUCTION && code->data[i + 1]->op == AST_OP_EXEC) { 
            auto body = code->data[i];

            code->unordered_remove(i + 1);
            code->unordered_remove(i);

            if(body->lambda.count) {
                code->extend_before(i, &body->lambda);
            }
        }
        i++;
    }
}

static void optimize_cond_execs(Allocator allocator, Array<Node*> *code) {
    u32 i = 0;
    while(i < code->count) {
        if(i + 1 < code->count &&
            code->data[i]->type == NODE_LAMBDA &&
            code->data[i + 1]->type == NODE_INSTRUCTION && code->data[i + 1]->op == AST_OP_COND_EXEC
        ) {
            auto body = code->data[i];

            code->ordered_remove(i + 1);
            code->ordered_remove(i);

            if(body->lambda.count == 0) {
                // TODO: We could do better here. For example, if the node before the lambda
                // is something we can evaluate as a constant at compile time, we don't have to
                // generate the code for the if statement; we already know whether it should execute or not.
                //                  - sci4me, Aug 21, 2019

                auto n = (Node*) ALLOC(allocator, sizeof(Node));
                n->type = NODE_INSTRUCTION;
                n->op = AST_OP_DROP;
                code->insert_before(i, n);
            } else {
                auto bn = (Node*) ALLOC(allocator, sizeof(Node));
                bn->type = NODE_INSTRUCTION;
                bn->op = AST_OP_BRANCH;
                bn->label = next_label++;

                auto bt = (Node*) ALLOC(allocator, sizeof(Node));
                bt->type = NODE_INSTRUCTION;
                bt->op = AST_OP_BRANCH_TARGET;
                bt->label = bn->label;

                auto ti = Array<Node*>();
                ti.push(bn);
                FOR((&body->lambda), j) {
                    ti.push(body->lambda.data[j]);
                }
                ti.push(bt);

                code->extend_before(i, &ti);
                // TODO: i += ti.count - 1 ?
            }
        }

        i++;
    }
}

static void optimize_while_loops(Allocator allocator, Array<Node*> *code) {
    u32 i = 0;
    while(i < code->count) {
        if(i + 2 < code->count &&
            code->data[i + 2]->type == NODE_INSTRUCTION && code->data[i + 2]->op == AST_OP_WHILE) {
            /*

            casees:
                - body known, cond known
                - body known, cond not
                - body not,   cond known
                - body not,   cond not

            we should be able to generate code for all of these cases

            */
        }
        
        i++;
    }
}

void optimize(Allocator allocator, Array<Node*> *code) {
    FOR(code, i) {
        if(code->data[i]->type == NODE_LAMBDA) optimize(allocator, &code->data[i]->lambda);
    }

    optimize_execs(allocator, code);
    optimize_cond_execs(allocator, code);
    optimize_while_loops(allocator, code);
}
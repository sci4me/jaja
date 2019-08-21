#include <stdio.h>

#include "optimizer.h"
#include "runtime.h"

// Theoretically we can do better than this; there are cases where we know what something is at compile time
// but just don't have easy access to that information for whatever reason i.e. it's a native function that exists
// within the global scope. But if/when scopes change, that will likely effect this. *shrug*
 //                  - sci4me, Aug 21, 2019

static u64 next_label = 0;

static void optimize_execs(Allocator allocator, Array<Node*> *code) {
    u32 i = 0;
    while(i < code->count) {
        if(i + 1 < code->count &&
            code->data[i]->type == NODE_LAMBDA &&
            code->data[i + 1]->type == NODE_INSTRUCTION && code->data[i + 1]->op == AST_OP_EXEC) { 
            auto body = code->data[i];

            FREE(allocator, code->data[i]);
            FREE(allocator, code->data[i + 1]);

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

            FREE(allocator, code->data[i]);
            FREE(allocator, code->data[i + 1]);

            code->ordered_remove(i + 1);
            code->ordered_remove(i);

            auto ti = Array<Node*>();

            if(body->lambda.count == 0) {
                // TODO: We could do better here. For example, if the node before the lambda
                // is something we can evaluate as a constant at compile time, we don't have to
                // generate the code for the if statement; we already know whether it should execute or not.
                //                  - sci4me, Aug 21, 2019

                auto n = (Node*) ALLOC(allocator, sizeof(Node));
                n->type = NODE_INSTRUCTION;
                n->op = AST_OP_DROP;
                ti.push(n);
            } else {
                auto bn = (Node*) ALLOC(allocator, sizeof(Node));
                bn->type = NODE_INSTRUCTION;
                bn->op = AST_OP_BRANCH_IF_FALSE;
                bn->label = next_label++;

                auto bt = (Node*) ALLOC(allocator, sizeof(Node));
                bt->type = NODE_INSTRUCTION;
                bt->op = AST_OP_BRANCH_TARGET;
                bt->label = bn->label;

                ti.push(bn);
                FOR((&body->lambda), j) {
                    ti.push(body->lambda.data[j]);
                }
                ti.push(bt);
            }

            code->extend_before(i, &ti);
            i += ti.count - 1;
    
            assert(code->data[i]->type == NODE_INSTRUCTION && code->data[i]->op == AST_OP_BRANCH_TARGET);
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
            however, for now, we'll only handle the first case

            */
            if(code->data[i]->type == NODE_LAMBDA && code->data[i + 1]->type == NODE_LAMBDA) {
                auto cond = code->data[i];
                auto body = code->data[i + 1];

                FREE(allocator, code->data[i]);
                FREE(allocator, code->data[i + 1]);
                FREE(allocator, code->data[i + 2]);

                code->ordered_remove(i + 2);
                code->ordered_remove(i + 1);
                code->ordered_remove(i);
            
                auto ti = Array<Node*>();

                auto check = (Node*) ALLOC(allocator, sizeof(Node));
                check->type = NODE_INSTRUCTION;
                check->op = AST_OP_BRANCH_TARGET;
                check->label = next_label++;
                ti.push(check);

                FOR((&cond->lambda), i) {
                    ti.push(cond->lambda.data[i]);
                }

                auto post_cond = (Node*) ALLOC(allocator, sizeof(Node));
                post_cond->type = NODE_INSTRUCTION;
                post_cond->op = AST_OP_BRANCH_IF_FALSE;
                post_cond->label = next_label++;
                ti.push(post_cond);

                FOR((&body->lambda), i) {
                    ti.push(body->lambda.data[i]);
                }

                {
                    auto x = (Node*) ALLOC(allocator, sizeof(Node));
                    x->type = NODE_INSTRUCTION;
                    x->op = AST_OP_BRANCH;
                    x->label = check->label;
                    ti.push(x);
                }

                {
                    auto x = (Node*) ALLOC(allocator, sizeof(Node));
                    x->type = NODE_INSTRUCTION;
                    x->op = AST_OP_BRANCH_TARGET;
                    x->label = post_cond->label;
                    ti.push(x);
                }

                code->extend_before(i, &ti);
            }
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
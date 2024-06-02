#Ilyas Umalatov X7278165E
all: mcp_bb

mcp_greedy: mcp_bb.cc
    g++ -O3 -o mcp_bb mcp_bb.cc
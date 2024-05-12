#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <iostream>
#include <sstream>
#include <memory>
#include <functional>

typedef std::string Word;

struct Match
{
    Word word;
    size_t begin_index;
    std::string *in_string;
};

struct ACTreeNode
{
    typedef std::shared_ptr<ACTreeNode> Ptr;
    typedef std::pair<const char, Ptr> Child;
    typedef std::unordered_map<char, Ptr> Children;
    Children children;
    std::string const *word = nullptr;
    ACTreeNode::Ptr fail;
};

ACTreeNode::Ptr build_trie(std::vector<std::string> &words)
{
    ACTreeNode::Ptr root = std::make_shared<ACTreeNode>();
    for (std::string &word: words)
    {
        ACTreeNode::Ptr node = root;
        for (char c: word)
        {
            if (node->children.count(c) == 0)
            {
                node->children[c] = std::make_unique<ACTreeNode>();
            }
            node = node->children[c];
        }
        node->word = &word;
    }

    std::queue<ACTreeNode::Ptr> q;
    q.push(root);
    while (!q.empty())
    {
        ACTreeNode::Ptr node = q.front();
        q.pop();
        for (const auto &[c, child]: node->children)
        {
            ACTreeNode::Ptr fail = node->fail;
            while (fail != nullptr && fail->children.count(c) == 0)
            {
                fail = fail->fail;
            }
            if (fail == nullptr)
            {
                child->fail = root;
            }
            else
            {
                child->fail = fail->children[c];
            }
            if (child->word != nullptr)
            {
                q.push(child);
            }
        }
    }
    return root;
}

std::vector<Match> find_all_positions(std::string &text, const ACTreeNode::Ptr &root)
{
    std::vector<Match> results;
    ACTreeNode::Ptr node = root;
    for (size_t i = 0; i < text.size(); i++)
    {
        char c = text[i];
        while (node != nullptr && node->children.count(c) == 0)
        {
            node = node->fail;
        }
        if (node == nullptr)
        {
            node = root;
        }
        else
        {
            node = node->children[c];
            if (node->word != nullptr)
            {
                results.push_back(Match{*node->word, i - node->word->size() + 1, &text});
            }
        }
    }
    return results;
}

std::string trie_to_dot(const ACTreeNode::Ptr &root)
{
    std::stringstream ss;
    ss << "digraph trie {" << std::endl;
    ss << "  node [shape=circle];" << std::endl;
    std::function<void(ACTreeNode::Ptr, int)> dfs = [&](const ACTreeNode::Ptr &node, int id) {
        ss << "  node_" << id << " [label=\"" << (node->word ? *node->word : ".") << "\"];" << std::endl;
        for (const ACTreeNode::Child &child: node->children)
        {
            ss << "  node_" << id << " -> node_" << (id + 1) << " [label=\"" << child.first << "\"];" << std::endl;
            dfs(child.second, id + 1);
        }
    };
    dfs(root, 0);
    ss << "}" << std::endl;
    return ss.str();
}

int main()
{
    bool print_tree = true;

    std::vector<std::string> words = {"fuck", "fucking", "ass", "shit", "moron"};
    std::string text = "this is really really simple, but i'm telling you should the shut the fuck up, "
                       "you fucking moron, and get your ass out of here";
    ACTreeNode::Ptr trie = build_trie(words);
    std::vector<Match> matches = find_all_positions(text, trie);
    if (print_tree)
    {
        std::cout << trie_to_dot(trie);
    }
    for (const auto &match: matches)
    {
        std::cout << "Word: " << match.word << ", Start index: " << match.begin_index << std::endl;
    }
    return 0;
}


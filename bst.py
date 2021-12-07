import sys


class TreeNode:
    def __init__(self, key):
        self.key = key
        self.left = None
        self.right = None


class BinarySearchTree:
    def __init__(self, root=None):
        self.root = root
        self.size = 0

    # Find the node with the maximum value
    def get_min_node(self):
        return self.__get_min_node(self.root)

    # Find the node with the maximum value
    def get_max_node(self):
        return self.__get_max_node(self.root)

    # Insert a node with key into the BST
    def add_value(self, key):
        self.root = self.__add_value(self.root, key)

    def remove_value(self, key):
        self.root = self.__remove_value(self.root, key)

    def ceil(self, key):
        return self.__ceil(self.root, key)

    def floor(self, key):
        return self.__floor(self.root, key)

    def contain(self, key):
        node = self.__get_node(self.root, key)
        if node is None:
            return False
        else:
            return True

    def __get_node(self, node, key):
        if node is None:
            return None
        if node.key == key:
            return node
        elif node.key > key:
            return self.__get_node(node.left, key)
        else:
            return self.__get_node(node.right, key)

    def __ceil(self, node, key):
        if node is None:
            return None
        # node's key is key
        if node.key == key:
            return node

        if node.key < key:
            return self.__ceil(node.right, key)

        tmp = self.__ceil(node.left, key)
        if tmp is not None:
            return tmp
        return node

    def __floor(self, node, key):
        if node is None:
            return None
        # node's key is key
        if node.key == key:
            return node

        if node.key > key:
            return self.__floor(node.left, key)

        tmp = self.__floor(node.right, key)
        if tmp is not None:
            return tmp
        return node

    def __get_min_node(self, node):
        if node is None:
            return None
        if node.left is None:
            return node
        return self.__get_min_node(node.left)

    def __get_max_node(self, node):
        if node is None:
            return None
        if node.right is None:
            return node
        return self.__get_max_node(node.right)

    def __add_value(self, node, key):
        if node is None:
            self.size += 1
            return TreeNode(key)
        if node.key < key:
            node.right = self.__add_value(node.right, key)
        else:
            node.left = self.__add_value(node.left, key)
        return node

    def __remove_value(self, node, key):
        parent = None
        cur = node
        while cur is not None and cur.key != key:
            parent = cur
            if cur.key > key:
                cur = cur.left
            elif cur.key < key:
                cur = cur.right

        if cur is None:
            print("no found")
            return node

        self.size -= 1
        if parent is None:
            return self.__remove_root(cur)

        if parent.left == cur:
            parent.left = self.__remove_root(cur)
        elif parent.right == cur:
            parent.right = self.__remove_root(cur)

        return node

    def __remove_root(self, node):
        if node is None:
            return None
        if node.left is None:
            return node.right
        if node.right is None:
            return node.left

        successor = self.__get_min_node(node.right)
        successor.right = self.__remove_min_node(node.right)
        successor.left = node.left
        return successor

    def __remove_min_node(self, node):
        if node is None:
            return None
        if node.left is None:
            return node.right
        node.left = self.__remove_min_node(node.left)
        return node

    def height(self):
        return self.__height(self.root)

    def __height(self, node):
        if node is None:
            return 0
        return max(self.__height(node.left), self.__height(node.right)) + 1

    def __len__(self):
        return self.size


def mid_traverse(root):
    if root is None:
        return
    mid_traverse(root.left)
    print(root.key, ' ', end='')
    mid_traverse(root.right)


def test():
    # 点击右上角，或按 Ctrl+Enter 来运行这段代码
    print('定义 BinarySearchTree 成功')

    bst = BinarySearchTree()
    for num in (7, 5, 9, 8, 15, 16, 18, 17, 7):
        bst.add_value(num)
    max_node = bst.get_max_node()
    min_node = bst.get_min_node()
    mid_traverse(bst.root)
    print("len = ", len(bst))
    print(bst.height())

    print(f"Max node: {max_node.key}")
    print(f"Min node: {min_node.key}")

    bst.remove_value(7)
    bst.remove_value(100)

    mid_traverse(bst.root)
    print("len = ", len(bst))

    print(bst.ceil(1).key)
    print(bst.floor(13).key)
    print(bst.ceil(13).key)


    print(bst.contain(15))
    print(bst.contain(2))


def main():
    test()
    psize = len(sys.argv)
    if psize == 1:
        print("no para")
    elif psize == 2:
        print("psize = 2")
        print(sys.argv[-1])
    elif psize == 3:
        print("psize = 3")
        print(sys.argv)
    else:
        print("psize = other")
        print(sys.argv)


if __name__ == "__main__":
    main()

/*************************************************************************
 * @version : 0.0.1
 * @date : 2023.03.03
 * @brief : the implements of twll
 * @copyright : Copyright 2022-2023 Quectel Inc. All Rights Reserved.
 *************************************************************************/

#ifndef __QUOS_TWLL_H__
#define __QUOS_TWLL_H__
#include "qhal_types.h"
#include "qhal_sdk_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct __Quos_twll_t
    {
        struct __Quos_twll_t *prev;
        struct __Quos_twll_t *next;
    } Quos_twll_t;

	/**
	 * Macro for iterating over a linked list.
	 * This macro provides a safe way to traverse a linked list, ensuring that the list
	 * can be modified (e.g., nodes removed) during iteration without causing undefined behavior.
	 *
	 * Parameters:
	 *   LISTHEAD - Pointer to the head of the linked list.
	 *   LISTTEMP - Temporary pointer used to iterate through the list.
	 *   NEXTLIST - Pointer to store the next node in the list, ensuring safe traversal.
	 *
	 * Usage:
	 *   The macro initializes LISTTEMP to the head of the list and NEXTLIST to the next node.
	 *   It continues the loop as long as LISTTEMP is not NULL, updating both pointers at each step.
	 */
#define TWLIST_FOR_DATA(LISTHEAD, LISTTEMP, NEXTLIST)                                \
	for ((LISTTEMP) = (LISTHEAD), (NEXTLIST) = (LISTHEAD) ? (LISTHEAD)->next : NULL; \
		 NULL != (LISTTEMP);                                                         \
		 (LISTTEMP) = (NEXTLIST), (NEXTLIST) = (NEXTLIST) ? (NEXTLIST)->next : NULL)

	/**
	 * Adds a node to the tail of a doubly linked list.
	 *
	 * This function first removes the node from its current position in the list (if it exists),
	 * then adds it to the tail of the list. If the list is empty, the node becomes the head of the list.
	 *
	 * @param list A pointer to the head pointer of the doubly linked list.
	 * @param node The node to be added to the tail of the list.
	 */
	void Quos_twllAdd(Quos_twll_t **list, Quos_twll_t *node);

	/**
	 * Adds a node to the beginning of a circular doubly linked list.
	 *
	 * This function first removes the node from its current position in the linked list (if it exists),
	 * then inserts the node at the beginning of the list, and updates the corresponding pointers.
	 *
	 * @param list A pointer to the head node pointer of the circular doubly linked list.
	 * @param node The node to be added to the beginning of the list.
	 */
	void Quos_twllAddFirst(Quos_twll_t **list, Quos_twll_t *node);

	/**
	 * Quos_twllDelete function is used to delete a specified node from a double-linked list.
	 * This function does not free the memory of the deleted node, it only removes the node from the list.
	 *
	 * @param list Pointer to the head node pointer of the double-linked list, modified by this function to reflect the changes in the list structure.
	 * @param node Pointer to the node to be deleted, which must be a valid node in the list.
	 */
	void Quos_twllDelete(Quos_twll_t **list, Quos_twll_t *node);

	/**
	 * Inserts a node in front of a reference node in a doubly linked list.
	 *
	 * This function first deletes the node to be inserted from the list (if it exists), then inserts it in front of the reference node.
	 * If the reference node is the head of the list, the node to be inserted becomes the new head of the list.
	 *
	 * @param list Pointer to the pointer of the head node of the doubly linked list, used to update the head node pointer when necessary.
	 * @param referNode Pointer to the reference node, indicating where the node to be inserted should be placed in front of.
	 * @param node Pointer to the node to be inserted.
	 * @return Returns TRUE if the insertion is successful, otherwise returns FALSE.
	 */
	qbool Quos_twllInsertFront(Quos_twll_t **list, Quos_twll_t *referNode, Quos_twll_t *node);

	/**
	 * Inserts a node behind a reference node in a doubly linked list.
	 *
	 * This function first deletes the node to be inserted from the list (if it exists), then inserts it behind the reference node.
	 * If the reference node is found, the function adjusts the pointers of the surrounding nodes accordingly and returns TRUE.
	 * If the reference node is not found, the function returns FALSE.
	 *
	 * @param list Pointer to the head node pointer of the doubly linked list.
	 * @param referNode Pointer to the reference node, indicating where the new node should be inserted.
	 * @param node Pointer to the node to be inserted.
	 * @return Returns TRUE if the insertion is successful, otherwise returns FALSE.
	 */
	qbool Quos_twllInsertBehind(Quos_twll_t **list, Quos_twll_t *referNode, Quos_twll_t *node);

	/**
	 * @brief Finds a node in a linked list by its index.
	 *
	 * This function traverses a linked list to find the node at the specified index. The index is 0-based,
	 * meaning the first node in the list is at index 0. If the index is greater than or equal to the length
	 * of the list, the function returns NULL, indicating that the search has exceeded the range of the list.
	 *
	 * @param list The head node of the linked list.
	 * @param nodeId The index of the target node.
	 * @return Quos_twll_t* Returns a pointer to the target node, or NULL if not found.
	 */
	Quos_twll_t *Quos_twllFineNodeById(Quos_twll_t *list, quint32_t nodeId);

	/**
	 * @brief Get the number of nodes in a doubly linked list
	 *
	 * This function calculates the number of nodes in a doubly linked list by traversing
	 * the list from the head node to the end. It increments a counter for each node encountered.
	 *
	 * @param list Pointer to the head node of the list. If the list is empty, this parameter is NULL.
	 * @return quint32_t The number of nodes in the list.
	 */
	quint32_t Quos_twllGetNodeCount(Quos_twll_t *list);

#ifdef __cplusplus
}
#endif

#endif /* __QUOS_TWLL_H__ */

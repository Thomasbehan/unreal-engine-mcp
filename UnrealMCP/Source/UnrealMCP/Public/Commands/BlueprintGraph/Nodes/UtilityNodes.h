// Header for creating utility nodes (Print, CallFunction, Select, SpawnActor, Math)

#pragma once

#include "CoreMinimal.h"
#include "EdGraph/EdGraph.h"

class UK2Node;

/**
 * Creator for Unreal Blueprint utility nodes
 */
class FUtilityNodeCreator
{
public:
	/**
	 * Creates a Print node (K2Node_CallFunction for PrintString)
	 * @param Graph - The graph to add the node to
	 * @param Params - JSON parameters containing pos_x, pos_y, message
	 * @return The created node or nullptr on error
	 */
	static UK2Node* CreatePrintNode(UEdGraph* Graph, const TSharedPtr<class FJsonObject>& Params);

	/**
	 * Creates a Call Function node (K2Node_CallFunction)
	 * Supports both UClass static functions and Blueprint self-member functions
	 * @param Graph - The graph to add the node to
	 * @param Params - JSON parameters containing:
	 *   - pos_x, pos_y: Node position
	 *   - target_function: Function name to call
	 *   - target_class: (Optional) UClass path for static functions
	 *   - call_self: (Optional) If true, calls a Blueprint-defined function on self
	 * @return The created node or nullptr on error
	 */
	static UK2Node* CreateCallFunctionNode(UEdGraph* Graph, const TSharedPtr<class FJsonObject>& Params);

	/**
	 * Creates a Math operation node using KismetMathLibrary
	 * @param Graph - The graph to add the node to
	 * @param Params - JSON parameters containing:
	 *   - pos_x, pos_y: Node position
	 *   - operation: Math operation name
	 *
	 * Supported operations:
	 *   Float: Add, Subtract, Multiply, Divide, Sin, Cos, Abs, Sqrt, Power
	 *   Int: Add_IntInt, Subtract_IntInt, Multiply_IntInt
	 *   Vector: Add_VectorVector, Subtract_VectorVector, Multiply_VectorFloat
	 *   Compare: EqualEqual_FloatFloat, Less_FloatFloat, Greater_FloatFloat
	 *   Utility: Lerp, FClamp, RandomFloatInRange
	 *
	 * @return The created node or nullptr on error
	 */
	static UK2Node* CreateMathNode(UEdGraph* Graph, const TSharedPtr<class FJsonObject>& Params);

	/**
	 * Creates a Select node (K2Node_Select)
	 * @param Graph - The graph to add the node to
	 * @param Params - JSON parameters containing pos_x, pos_y, pin_type
	 * @return The created node or nullptr on error
	 */
	static UK2Node* CreateSelectNode(UEdGraph* Graph, const TSharedPtr<class FJsonObject>& Params);

	/**
	 * Creates a Spawn Actor From Class node (K2Node_SpawnActorFromClass)
	 * @param Graph - The graph to add the node to
	 * @param Params - JSON parameters containing pos_x, pos_y, actor_class
	 * @return The created node or nullptr on error
	 */
	static UK2Node* CreateSpawnActorNode(UEdGraph* Graph, const TSharedPtr<class FJsonObject>& Params);

private:
	/**
	 * Helper to map operation names to KismetMathLibrary function names
	 */
	static FName GetMathFunctionName(const FString& Operation);
};

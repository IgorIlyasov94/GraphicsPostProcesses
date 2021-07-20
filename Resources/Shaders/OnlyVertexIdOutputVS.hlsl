struct Input
{
	uint vertexId : SV_VertexId;
};

struct Output
{
	uint vertexId : VERTEXID;
};

Output main(Input input)
{
	Output output = (Output)0;
	
	output.vertexId = input.vertexId;
	
	return output;
}

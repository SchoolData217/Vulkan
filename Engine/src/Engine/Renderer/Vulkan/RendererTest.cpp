#include "epch.h"
#include "RendererTest.h"

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <vulkan/vk_layer.h>
#include <vulkan/vulkan_win32.h>

#include <glm/glm.hpp>
#include <array>

using namespace glm;

namespace {

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 color;
	};

}

void RendererTest::prepare()
{
#if 1
	Vertex vertices[] =
	{
		{ vec3(-0.5f, -0.5f, 0.0f), {1.0f, 0.0f, 0.0f} },
		{ vec3(0.5f, -0.5f, 0.0f), {0.0f, 1.0f, 0.0f} },
		{ vec3(0.0f,  0.5f, 0.0f),  {0.0f, 0.0f, 1.0f} },
	};
	uint32_t indices[] = { 0, 1, 2 };

	m_vertexBuffer = createBuffer(sizeof(vertices), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	m_indexBuffer = createBuffer(sizeof(indices), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	// ���_�f�[�^�̏�������
	{
		void* p;
		vkMapMemory(m_Context.m_Device->m_LogicalDevice, m_vertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
		memcpy(p, vertices, sizeof(vertices));
		vkUnmapMemory(m_Context.m_Device->m_LogicalDevice, m_vertexBuffer.memory);
	}
	// �C���f�b�N�X�f�[�^�̏�������
	{
		void* p;
		vkMapMemory(m_Context.m_Device->m_LogicalDevice, m_indexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &p);
		memcpy(p, indices, sizeof(indices));
		vkUnmapMemory(m_Context.m_Device->m_LogicalDevice, m_indexBuffer.memory);
	}
	m_indexCount = _countof(indices);

	// ���_�̓��͐ݒ�
	VkVertexInputBindingDescription inputBinding{
		0,                          // binding
		sizeof(Vertex),          // stride
		VK_VERTEX_INPUT_RATE_VERTEX // inputRate
	};
	std::array<VkVertexInputAttributeDescription, 2> inputAttribs{
		{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos)},
		{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},
		}
	};
	VkPipelineVertexInputStateCreateInfo vertexInputCI{};
	vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCI.vertexBindingDescriptionCount = 1;
	vertexInputCI.pVertexBindingDescriptions = &inputBinding;
	vertexInputCI.vertexAttributeDescriptionCount = uint32_t(inputAttribs.size());
	vertexInputCI.pVertexAttributeDescriptions = inputAttribs.data();

	// �u�����f�B���O�̐ݒ�
	const auto colorWriteAll = \
		VK_COLOR_COMPONENT_R_BIT | \
		VK_COLOR_COMPONENT_G_BIT | \
		VK_COLOR_COMPONENT_B_BIT | \
		VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendAttachmentState blendAttachment{};
	blendAttachment.blendEnable = VK_TRUE;
	blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	blendAttachment.colorWriteMask = colorWriteAll;
	VkPipelineColorBlendStateCreateInfo cbCI{};
	cbCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cbCI.attachmentCount = 1;
	cbCI.pAttachments = &blendAttachment;

	// �r���[�|�[�g�̐ݒ�
	VkViewport viewport;
	{
		viewport.x = 0.0f;
		viewport.y = float(m_SwapChain.m_Height);
		viewport.width = float(m_SwapChain.m_Width);
		viewport.height = -1.0f * float(m_SwapChain.m_Height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
	}
	VkRect2D scissor = {
		{ 0,0},// offset
		{m_SwapChain.m_Width, m_SwapChain.m_Height}
	};
	VkPipelineViewportStateCreateInfo viewportCI{};
	viewportCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportCI.viewportCount = 1;
	viewportCI.pViewports = &viewport;
	viewportCI.scissorCount = 1;
	viewportCI.pScissors = &scissor;

	// �v���~�e�B�u�g�|���W�[�ݒ�
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
	inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;


	// ���X�^���C�U�[�X�e�[�g�ݒ�
	VkPipelineRasterizationStateCreateInfo rasterizerCI{};
	rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerCI.cullMode = VK_CULL_MODE_NONE;
	rasterizerCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizerCI.lineWidth = 1.0f;

	// �}���`�T���v���ݒ�
	VkPipelineMultisampleStateCreateInfo multisampleCI{};
	multisampleCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// �f�v�X�X�e���V���X�e�[�g�ݒ�
	VkPipelineDepthStencilStateCreateInfo depthStencilCI{};
	depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCI.depthTestEnable = VK_TRUE;
	depthStencilCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilCI.depthWriteEnable = VK_TRUE;
	depthStencilCI.stencilTestEnable = VK_FALSE;

	// �V�F�[�_�[�o�C�i���̓ǂݍ���
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages
	{
		loadShaderModule("Resources/Shaders/shader.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		loadShaderModule("Resources/Shaders/shader.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	// �p�C�v���C�����C�A�E�g
	VkPipelineLayoutCreateInfo pipelineLayoutCI{};
	pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkCreatePipelineLayout(m_Context.m_Device->m_LogicalDevice, &pipelineLayoutCI, nullptr, &m_pipelineLayout);

	// �p�C�v���C���̍\�z
	VkGraphicsPipelineCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	ci.stageCount = uint32_t(shaderStages.size());
	ci.pStages = shaderStages.data();
	ci.pInputAssemblyState = &inputAssemblyCI;
	ci.pVertexInputState = &vertexInputCI;
	ci.pRasterizationState = &rasterizerCI;
	ci.pDepthStencilState = &depthStencilCI;
	ci.pMultisampleState = &multisampleCI;
	ci.pViewportState = &viewportCI;
	ci.pColorBlendState = &cbCI;
	ci.renderPass = m_SwapChain.m_RenderPass;
	ci.layout = m_pipelineLayout;
	vkCreateGraphicsPipelines(m_Context.m_Device->m_LogicalDevice, VK_NULL_HANDLE, 1, &ci, nullptr, &m_pipeline);

	// ShaderModule �͂����s�v�̂��ߔj��
	for (const auto& v : shaderStages)
	{
		vkDestroyShaderModule(m_Context.m_Device->m_LogicalDevice, v.module, nullptr);
	}
#endif
}

void RendererTest::cleanup()
{
	vkDestroyPipelineLayout(m_Context.m_Device->m_LogicalDevice, m_pipelineLayout, nullptr);
	vkDestroyPipeline(m_Context.m_Device->m_LogicalDevice, m_pipeline, nullptr);

	vkFreeMemory(m_Context.m_Device->m_LogicalDevice, m_vertexBuffer.memory, nullptr);
	vkFreeMemory(m_Context.m_Device->m_LogicalDevice, m_indexBuffer.memory, nullptr);
	vkDestroyBuffer(m_Context.m_Device->m_LogicalDevice, m_vertexBuffer.buffer, nullptr);
	vkDestroyBuffer(m_Context.m_Device->m_LogicalDevice, m_indexBuffer.buffer, nullptr);
}

void RendererTest::makeCommand(VkCommandBuffer command)
{
	// �쐬�����p�C�v���C�����Z�b�g
	vkCmdBindPipeline(command, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

	// �e�o�b�t�@�I�u�W�F�N�g�̃Z�b�g
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(command, 0, 1, &m_vertexBuffer.buffer, &offset);
	vkCmdBindIndexBuffer(command, m_indexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32);

	// 3�p�`�`��
	vkCmdDrawIndexed(command, m_indexCount, 1, 0, 0, 0);
}

BufferObject RendererTest::createBuffer(uint32_t size, VkBufferUsageFlags usage)
{
	BufferObject obj;
	VkBufferCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	ci.usage = usage;
	ci.size = size;
	auto result = vkCreateBuffer(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &obj.buffer);
	VK_CHECK_RESULT(result);

	// �������ʂ̎Z�o
	VkMemoryRequirements reqs;
	vkGetBufferMemoryRequirements(m_Context.m_Device->m_LogicalDevice, obj.buffer, &reqs);
	VkMemoryAllocateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	info.allocationSize = reqs.size;
	// �������^�C�v�̔���
	auto flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	info.memoryTypeIndex = getMemoryTypeIndex(reqs.memoryTypeBits, flags);
	// �������̊m��
	vkAllocateMemory(m_Context.m_Device->m_LogicalDevice, &info, nullptr, &obj.memory);

	// �������̃o�C���h
	vkBindBufferMemory(m_Context.m_Device->m_LogicalDevice, obj.buffer, obj.memory, 0);
	return obj;
}

VkPipelineShaderStageCreateInfo RendererTest::loadShaderModule(const char* fileName, VkShaderStageFlagBits stage)
{
	std::ifstream infile(fileName, std::ios::binary);
	if (!infile)
	{
		OutputDebugStringA("file not found.\n");
		DebugBreak();
	}
	std::vector<char> filedata;
	filedata.resize(uint32_t(infile.seekg(0, std::ifstream::end).tellg()));
	infile.seekg(0, std::ifstream::beg).read(filedata.data(), filedata.size());

	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.pCode = reinterpret_cast<uint32_t*>(filedata.data());
	ci.codeSize = filedata.size();
	vkCreateShaderModule(m_Context.m_Device->m_LogicalDevice, &ci, nullptr, &shaderModule);

	VkPipelineShaderStageCreateInfo shaderStageCI{};
	shaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStageCI.stage = stage;
	shaderStageCI.module = shaderModule;
	shaderStageCI.pName = "main";
	return shaderStageCI;
}
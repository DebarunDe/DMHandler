import aws_cdk as core
import aws_cdk.assertions as assertions

from cdk_dmhandler_pipeline.cdk_dmhandler_pipeline_stack import CdkDmhandlerPipelineStack

# example tests. To run these tests, uncomment this file along with the example
# resource in cdk_dmhandler_pipeline/cdk_dmhandler_pipeline_stack.py
def test_sqs_queue_created():
    app = core.App()
    stack = CdkDmhandlerPipelineStack(app, "cdk-dmhandler-pipeline")
    template = assertions.Template.from_stack(stack)

#     template.has_resource_properties("AWS::SQS::Queue", {
#         "VisibilityTimeout": 300
#     })

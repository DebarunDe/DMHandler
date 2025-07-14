from aws_cdk import (
    # Duration,
    Stack,
    aws_s3 as s3,
    aws_s3_deployment as s3deploy,
    aws_lambda as _lambda,
    aws_s3_notifications as s3n,
    aws_iam as iam,
    # aws_sqs as sqs,
)
from constructs import Construct

class CdkDmhandlerPipelineStack(Stack):

    def __init__(self, scope: Construct, construct_id: str, **kwargs) -> None:
        super().__init__(scope, construct_id, **kwargs)

        # The code that defines your stack goes here

        # example resource
        # queue = sqs.Queue(
        #     self, "CdkDmhandlerPipelineQueue",
        #     visibility_timeout=Duration.seconds(300),
        # )

        # Import s3 Bucket
        bucket = s3.Bucket.from_bucket_name(
            self, "ImportedBucket",
            bucket_name="dmhandler-build-artifacts"
        )

        lambda_fn = _lambda.Function(
            self, "DmHandlerLambda",
            function_name="DmHandlerLambda",
            runtime=_lambda.Runtime.PYTHON_3_9,
            handler="lambda_handler.main",
            code=_lambda.Code.from_asset("lambda")
        )

        bucket.grant_read(lambda_fn)

        lambda_fn.add_permission(
            "AllowS3Invoke",
            principle=iam.ServicePrincipal("s3.amazonaws.com"),
            source_arn=bucket.bucket_arn,
        )

        #add event notification to bucket
        notification = s3n.LambdaDestination(lambda_fn)
        bucket.add_event_notification(
            s3.EventType.OBJECT_CREATED,
            notification,
            s3.NotificationKeyFilter(prefix="release/")
        )
